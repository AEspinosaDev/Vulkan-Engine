//////////////////////////////////////////////
//MULTIPLE SAMPLING AND SPACE WARPING FUNCTIONS
//////////////////////////////////////////////
#ifndef PI 
#define PI           3.1415926535897932384626433832795
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

vec3 cosineWeightedHemisphereSample(vec2 rng) {
    float r = sqrt(rng.x);
    float theta = 6.283 * rng.y;
    float x = r * cos(theta);
    float y = r * sin(theta);
    return vec3(x, y, sqrt(max(0.0, 1.0 - rng.x)));
}

// vec3 cosineWeightedHemisphereSample(vec2 rng) {
//     float r = sqrt(rng.x);             
//     float theta = 6.283 * rng.y;          
//     float x = r * cos(theta);
//     float y = r * sin(theta);
//     float z = sqrt(max(0.0, 1.0 - r * r));
//     return vec3(x, y, z);  // return the cosine-weighted hemisphere sample
// }