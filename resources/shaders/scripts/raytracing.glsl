#ifndef PI 
#define PI              3.1415926535897932384626433832795
#endif
#ifndef EPSILON 
#define EPSILON         0.001
#endif  
#ifndef GOLDEN_RATIO 
#define GOLDEN_RATIO 2.118033988749895
#endif  

vec2 whiteNoiseSample(vec3 p3) {
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

vec4 blueNoiseSample(sampler2D blueNoiseMap,int i, int ditherFactor) {
    vec2 blueNoiseSize = textureSize(blueNoiseMap, 0);
    ivec2 fragUV = ivec2(mod(gl_FragCoord.xy, blueNoiseSize));
    return fract(texelFetch(blueNoiseMap, fragUV, 0)+ GOLDEN_RATIO *(128*i + ditherFactor) );
	
}

vec2 diskSample(vec2 rng, float radius) {
    float pointRadius = radius*sqrt(rng.x);
    float pointAngle = rng.y * 2.0f * PI;
    return vec2(pointRadius*cos(pointAngle), pointRadius*sin(pointAngle));
}

float computeRaytracedShadow(accelerationStructureEXT TLAS, sampler2D blueNoiseMap, vec3 O, vec3 L, int numSamples, float area, int ditherFactor){
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
		vec2 rng = blueNoiseSample(blueNoiseMap,i,ditherFactor).rg;
		vec2 dsample = diskSample(rng,radius);

		float tMax = length(L);
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
