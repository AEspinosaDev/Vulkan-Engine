//////////////////////////////////////////////
// ATTENTTION
// This script needs: scene.glsl, utils.glsl, 
//	cook_torrance_BRDF.glsl, fresnel.glsl
//////////////////////////////////////////////

#ifndef MIPMAP_HARDCAP
#define MIPMAP_HARDCAP 9.0
#endif

#ifndef SQRT2
#define SQRT2 1.414213
#endif

// #define HIGH_QUALITY_VXGI

//Settings
struct VXGI {
	float 	strength;
	float 	diffuseConeSpread;
	float 	offset;
	float 	maxDistance;
	uint 	resolution;
	uint 	samples;
	uint 	enabled;
	uint 	updateMode;
};
// Randomly Uniformly Dist. generated cone directions
#ifdef HIGH_QUALITY_VXGI
const int DIFFUSE_CONE_COUNT 			= 16;
const vec3 DIFFUSE_CONE_DIRECTIONS[16] 	= {
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
#else
const int DIFFUSE_CONE_COUNT = 8;
const vec3 DIFFUSE_CONE_DIRECTIONS[8] = {
    vec3(0.57735, 0.57735, 0.57735),   // Top-center
    vec3(0.57735, -0.57735, 0.57735),  // Bottom-center
    vec3(-0.57735, 0.57735, 0.57735),  // Top-left
    vec3(-0.57735, -0.57735, 0.57735), // Bottom-left
    vec3(0.57735, 0.57735, -0.57735),  // Top-right
    vec3(0.57735, -0.57735, -0.57735), // Bottom-right
    vec3(-0.57735, 0.57735, -0.57735), // Top-back
    vec3(-0.57735, -0.57735, -0.57735) // Bottom-back
};
#endif

vec4 traceCone(sampler3D voxelization, vec3 origin, vec3 direction, const float MAX_DISTANCE ,const float CONE_SPREAD, const float VOXEL_WORLD_SIZE)
{
	const float ALPHA_THRESHOLD = 0.95;
	
	vec3 color 		= vec3(0.0);
	float alpha 	= 0.0;
	float occlusion = 0.0;
	float dist 		= VOXEL_WORLD_SIZE;

	while( alpha < ALPHA_THRESHOLD)
	{
		float diameter 	= max(VOXEL_WORLD_SIZE, 2.0 * CONE_SPREAD * dist);
		float lodLevel 	= log2(diameter / VOXEL_WORLD_SIZE);
		vec3 voxelCoord = mapToZeroOne(origin + dist * direction, scene.minCoord.xyz, scene.maxCoord.xyz);
		vec4 voxelColor = textureLod(voxelization, voxelCoord,  min(MIPMAP_HARDCAP, lodLevel));


		color 		+= (1.0 - alpha) * voxelColor.rgb;
		occlusion 	+= ((1.0 - alpha) * voxelColor.a) / (1.0 + 0.03 * diameter);
		alpha 		+= (1.0 - alpha) * voxelColor.a;
		dist 		+= diameter;
	}

	return vec4(color, occlusion);
}

vec4 diffuseVoxelGI(sampler3D voxelMap, vec3 worldPos, vec3 worldNormal, VXGI vxgi, float worldVolumeSize){
	const float VOXEL_SIZE  = 1.0/float(vxgi.resolution);
    const float ANGLE_MIX 	= 0.5; 
    const float CONE_SPREAD = vxgi.diffuseConeSpread;
	const int SAMPLES 		= 4;

	// const float w[SAMPLES+1] = {.2,.1, .1, .1, .1,.1,.1,.1,.1};  // Cone weights. Importance sampling on pole
	const float w[5] 		= {.28, .18, .18, .18,.18};  // Cone weights. Importance sampling on pole

	// Find a base for the side cones with the normal as one of its base vectors.
	const vec3 ortho 	= normalize(orthogonal(worldNormal));
	const vec3 ortho2 	= normalize(cross(ortho, worldNormal));
	const vec3 corner 	= 0.5 * (ortho + ortho2);
	const vec3 corner2 	= 0.5 * (ortho - ortho2);
	// Direction for the side cones
	// const vec3 sDirs[SAMPLES] ={
	// 	mix(worldNormal, ortho, 	ANGLE_MIX),
	// 	mix(worldNormal, -ortho, 	ANGLE_MIX),
	// 	mix(worldNormal, ortho2, 	ANGLE_MIX),
	// 	mix(worldNormal, -ortho, 	ANGLE_MIX),
	// 	mix(worldNormal, corner, 	ANGLE_MIX),
	// 	mix(worldNormal, -corner, 	ANGLE_MIX),
	// 	mix(worldNormal, corner2, 	ANGLE_MIX),
	// 	mix(worldNormal, -corner2, 	ANGLE_MIX),
	// };
	const vec3 sDirs[4] ={
		mix(worldNormal, ortho, 	ANGLE_MIX),
		mix(worldNormal, -ortho, 	ANGLE_MIX),
		mix(worldNormal, ortho2, 	ANGLE_MIX),
		mix(worldNormal, -ortho, 	ANGLE_MIX),
	};

	//Offset start position
	float voxelWorldSize = worldVolumeSize / float(vxgi.resolution);
	vec3 startPos = worldPos + worldNormal * voxelWorldSize * vxgi.offset;

	// Accumulate indirect diffuse light.
	vec4 indirectDiffuse = vec4(0.0);

	// Trace front cone
	indirectDiffuse += w[0] * traceCone(voxelMap, startPos, worldNormal, vxgi.maxDistance, CONE_SPREAD, voxelWorldSize);
	
	// Trace side cones
	for(int i = 0; i < SAMPLES; i++){
		float cosTheta = dot(worldNormal, sDirs[i]);
		indirectDiffuse += w[i+1] * traceCone(voxelMap, startPos, sDirs[i], vxgi.maxDistance, CONE_SPREAD, voxelWorldSize ) * cosTheta;
	}

	indirectDiffuse.rgb*=vxgi.strength; 
	return indirectDiffuse; 
}

vec4 diffuseVoxelGI2(sampler3D voxelMap, vec3 worldPos, vec3 worldNormal, VXGI vxgi, float worldVolumeSize){
	
	const float DIFFUSE_CONE_APERTURE 	= vxgi.diffuseConeSpread;
    const float VOXEL_SIZE  			= 1.0/float(vxgi.resolution);

	vec4 diffuseIndirect = vec4(0.0);

	float voxelWorldSize 	= worldVolumeSize / float(vxgi.resolution);
	vec3 startPos 			= worldPos + worldNormal * voxelWorldSize * vxgi.offset;
    
    float coneTraceCount = 0.0;
	for (int i = 0; i < DIFFUSE_CONE_COUNT; ++i)
    {
		float cosTheta = dot(worldNormal, DIFFUSE_CONE_DIRECTIONS[i]);
        if (cosTheta < 0.0)
            continue;
        coneTraceCount += 1.0;

		diffuseIndirect += traceCone(voxelMap, startPos,  DIFFUSE_CONE_DIRECTIONS[i], vxgi.maxDistance, DIFFUSE_CONE_APERTURE, voxelWorldSize ) * cosTheta;
    }

	//Avrg
	diffuseIndirect /= coneTraceCount;
     
	diffuseIndirect.rgb*=vxgi.strength; 
	return diffuseIndirect; 

}
vec4 diffuseVoxelGI_CookTorrance(sampler3D voxelMap, samplerCube envMap, vec3 worldPos, vec3 worldNormal, vec3 worldView, VXGI vxgi, CookTorranceBRDF brdf, float worldVolumeSize){
	
	const float DIFFUSE_CONE_APERTURE 	= vxgi.diffuseConeSpread;
    const float VOXEL_SIZE  			= 1.0/float(vxgi.resolution);


	float voxelWorldSize 	= worldVolumeSize / float(vxgi.resolution);
	vec3 startPos 			= worldPos + worldNormal * voxelWorldSize * vxgi.offset;
    
    float coneTraceCount 	= 0.0;
	vec3 diffuseIndirect 	= vec3(0.0);
	float occlusion 		= 0.0;
	for (int i = 0; i < DIFFUSE_CONE_COUNT; ++i)
    {
		float cosTheta = dot(worldNormal, DIFFUSE_CONE_DIRECTIONS[i]);
        if (cosTheta < 0.0)
            continue;
        coneTraceCount += 1.0;

		vec4 irradiance  = traceCone(voxelMap, startPos,  DIFFUSE_CONE_DIRECTIONS[i], vxgi.maxDistance, DIFFUSE_CONE_APERTURE, voxelWorldSize);
		diffuseIndirect += evalDiffuseCookTorranceBRDF(DIFFUSE_CONE_DIRECTIONS[i], worldView ,irradiance.rgb, brdf) * cosTheta;
		occlusion 		+= irradiance.a;

		// diffuseIndirect += (1.0 - irradiance.a) *vec3(1.0,0.0,0.0) * scene.ambientIntensity; 
    }

	//Avrg
	diffuseIndirect /= coneTraceCount;
	occlusion  		/= coneTraceCount;
     
	return vec4(diffuseIndirect*vxgi.strength, occlusion); 

}

