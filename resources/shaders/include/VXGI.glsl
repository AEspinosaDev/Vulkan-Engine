#ifndef MIPMAP_HARDCAP
#define MIPMAP_HARDCAP 5.4
#endif

#ifndef SQRT2
#define SQRT2 1.414213
#endif

struct VXGI {
	float 	strength;
	uint 	resolution;
	uint 	samples;
	uint 	enabled;
};

vec3 traceDiffuseVoxelCone(sampler3D voxelization ,const vec3 O, vec3 dir, const float VOXEL_SIZE){
	dir = normalize(dir);
	
	const float CONE_SPREAD = 0.325;

	vec4 color = vec4(0.0);

	// Controls bleeding from close surfaces.
	// Low values look rather bad if using shadow cone tracing.
	// Might be a better choice to use shadow maps and lower this value.
	// float dist = 0.1953125;
	float dist = 0.9;

	// Trace.
	while(dist < SQRT2 && color.a < 1){
		// vec3 c = o + dist * dir;

        vec3 c = mapToZeroOne(O + dist * dir, scene.minCoord.xyz, scene.maxCoord.xyz);
		// c = scaleAndBias(o + dist * dir);
		float l = (1 + CONE_SPREAD * dist / VOXEL_SIZE);
		float level = log2(l);
		float ll = (level + 1) * (level + 1);
		vec4 voxel = textureLod(voxelization, c, min(MIPMAP_HARDCAP, level));

  		float weightOfNewSample = (1.0 - color.a);
        color += weightOfNewSample * voxel;

		// color += 0.075 * ll * voxel * pow(1 - voxel.a, 2);
		dist += ll * VOXEL_SIZE * 2;
	}
	// return pow(color.rgb * 2.0, vec3(1.5));
	return color.rgb;

	
}

// Returns a soft shadow blend by using shadow cone tracing.
// Uses 2 samples per step, so it's pretty expensive.
float traceShadowCone(sampler3D voxelization , vec3 O, vec3 dir, vec3 normal, const  float VOXEL_SIZE, float targetDistance){
	O += normal * 0.05; // Removes artifacts but makes self shadowing for dense meshes meh.

	float color = 0;

	float dist = 3 * VOXEL_SIZE;
	// I'm using a pretty big margin here since I use an emissive light ball with a pretty big radius in my demo scenes.
	const float STOP = targetDistance - 16 * VOXEL_SIZE;

	while(dist < STOP && color < 1){	
		vec3 c = mapToZeroOne(O + dist * dir, scene.minCoord.xyz, scene.maxCoord.xyz);
		float l = pow(dist, 2); // Experimenting with inverse square falloff for shadows.
		float s1 = 0.062 * textureLod(voxelization, c, 1 + 0.75 * l).a;
		float s2 = 0.135 * textureLod(voxelization, c, 4.5 * l).a;
		float s = s1 + s2;
		color += (1 - color) * s;
		dist += 0.9 * VOXEL_SIZE * (1 + 0.05 * l);
	}
	return 1 - pow(smoothstep(0, 1, color * 1.4), 1.0 / 1.4);
}