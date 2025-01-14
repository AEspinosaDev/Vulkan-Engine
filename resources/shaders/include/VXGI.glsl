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

const int DIFFUSE_CONE_COUNT = 16;
const float DIFFUSE_CONE_APERTURE = 0.872665;

const vec3 DIFFUSE_CONE_DIRECTIONS[16] = {
    vec3(0.57735, 0.57735, 0.57735),
    vec3(0.57735, -0.57735, -0.57735),
    vec3(-0.57735, 0.57735, -0.57735),
    vec3(-0.57735, -0.57735, 0.57735),
    vec3(-0.903007, -0.182696, -0.388844),
    vec3(-0.903007, 0.182696, 0.388844),
    vec3(0.903007, -0.182696, 0.388844),
    vec3(0.903007, 0.182696, -0.388844),
    vec3(-0.388844, -0.903007, -0.182696),
    vec3(0.388844, -0.903007, 0.182696),
    vec3(0.388844, 0.903007, -0.182696),
    vec3(-0.388844, 0.903007, 0.182696),
    vec3(-0.182696, -0.388844, -0.903007),
    vec3(0.182696, 0.388844, -0.903007),
    vec3(-0.182696, 0.388844, 0.903007),
    vec3(0.182696, -0.388844, 0.903007)
};

// vec4 traceVoxelCone(sampler3D voxelization, vec3 origin, vec3 direction, float tanHalfAngle)
// {
// 	float lod 		= 0.0f;
// 	vec3 color 		= vec3(0.0f);
// 	float alpha 	= 0.0f;
// 	float occlusion = 0.0f;

// 	//Voxel Cube Size
// 	float voxelWorldSize = VoxelGridWorldSize / VoxelDimensions;
// 	float dist = voxelWorldSize;
// 	vec3 startPos = Position_world + Normal_world * voxelWorldSize;

// 	while(dist < MAX_DISTANCE && alpha < MAX_ALPHA)
// 	{
// 		float diameter = max(voxelWorldSize, 2.0f * tanHalfAngle * dist);
// 		float lodLevel = log2(diameter / voxelWorldSize);
// 		vec4 voxelColor = SampleVoxels(startPos + dist * direction, lodLevel);

// 		color += (1.0f - alpha) * voxelColor.rgb;
// 		occlusion += ((1.0f - alpha) * voxelColor.a) / (1.0f + 0.03f * diameter);
// 		alpha += (1.0f - alpha) * voxelColor.a;
// 		dist += diameter;
// 	}

// 	return vec4(color, occlusion);
// }

/*
FIRST TRY
*/
vec3 traceVoxelCone(sampler3D voxelization ,const vec3 O, vec3 dir, const float CONE_SPREAD, const float VOXEL_SIZE){
	dir = normalize(dir);
	
	// const float CONE_SPREAD = 0.325;

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

vec3 diffuseVoxelGI(sampler3D voxelMap, vec3 worldPos, vec3 worldNormal, int resolution){
// 	vec3 diffuseIndirect = vec3(0.0);

//     const float VOXEL_SIZE  = 1.0/float(resolution);

    
//     vec3 startPos = worldPos + worldNormal * VOXEL_SIZE;

//     float coneTraceCount = 0.0;
//     float cosSum = 0.0;
// 	for (int i = 0; i < DIFFUSE_CONE_COUNT; ++i)
//     {
// 		float cosTheta = dot(worldNormal, DIFFUSE_CONE_DIRECTIONS[i]);
//         if (cosTheta < 0.0)
//             continue;
//         coneTraceCount += 1.0;
// 		diffuseIndirect += traceVoxelCone(voxelMap, startPos,  DIFFUSE_CONE_DIRECTIONS[i],DIFFUSE_CONE_APERTURE , VOXEL_SIZE ) * cosTheta;
//     }

// 	diffuseIndirect /= DIFFUSE_CONE_COUNT * 0.5;

//     // indirectContribution.a *= u_ambientOcclusionFactor;
//    return diffuseIndirect;

}
