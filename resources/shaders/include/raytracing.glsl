
//////////////////////////////////////////////
// ATTENTTION
// This script needs: warp.glsl
//////////////////////////////////////////////

#ifndef PI 
#define PI              3.1415926535897932384626433832795
#endif
#ifndef EPSILON 
#define EPSILON         0.001
#endif  

float computeRaytracedShadow(accelerationStructureEXT TLAS, sampler2D randomPool, vec3 O, vec3 L, int numSamples, float area, float tMax, int ditherFactor){
	if(numSamples == 0) return 1.0;

	//Shadow occlusion contribution
	float shadow = 0.0;
	//Create Light Frame
  	vec3 LTangent = normalize(cross(L, vec3(0, 1, 0)));
    vec3 LBitangent = normalize(cross(LTangent, L));
	//Disk radius
	float radius = area*0.5;
	for(int i = 0; i < numSamples; i++) {
		//Get random value
		vec2 rng = blueNoiseSample(randomPool,i,ditherFactor).rg;
		vec2 dsample = diskSample(rng,radius);

		// float tMax = length(L);
		vec3 dir = normalize(L + dsample.x * LTangent + dsample.y * LBitangent);

    	rayQueryEXT rayQuery;
		rayQueryInitializeEXT(rayQuery, TLAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, O, EPSILON, dir, tMax); 
		rayQueryProceedEXT(rayQuery);
		if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionTriangleEXT ) {
			shadow++;
		}
	}
	return 1.0 - (shadow / numSamples);

}

float computeRaytracedAO(accelerationStructureEXT TLAS, sampler2D randomPool, vec3 O, vec3 N, int numSamples, float tMin, float tMax, int ditherFactor) {
    if(numSamples == 0) return 1.0;

	//Ambient Occlusion contribution
    float occlusion = 0;
	//Create Surface Frame
    vec3 STangent = abs(N.z) > 0.5 ? vec3(0.0, -N.z, N.y) : vec3(-N.y, N.x, 0.0);
    vec3 SBitangent = cross(N, STangent);

    for(int i = 0; i < numSamples; i++) {

		//Get random value
		vec2 rng = blueNoiseSample(randomPool,i,ditherFactor).rg;
        vec3 randomVec = cosineWeightedHemisphereSample(rng);
        vec3 dir = STangent*randomVec.x + SBitangent*randomVec.y + N*randomVec.z;

		rayQueryEXT rayQuery;
		rayQueryInitializeEXT(rayQuery, TLAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, O, tMin, dir, tMax); 
		rayQueryProceedEXT(rayQuery);
        while(rayQueryProceedEXT(rayQuery)) {}
		if (rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionNoneEXT ) {
            occlusion += 1.0;
		}
       
    }
    return occlusion/numSamples;
}
