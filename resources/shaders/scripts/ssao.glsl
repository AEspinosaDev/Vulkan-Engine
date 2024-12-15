#ifndef MAX_SSAO_SAMPLES
#define MAX_SSAO_SAMPLES 64
#endif
// Classic SSAO
float SSAO(sampler2D vPosBuffer,
      vec3 vPos,
      vec3 vNormal, 
      vec3 randomDir,
      vec4 samples[MAX_SSAO_SAMPLES],
      const int KERNEL_SIZE, 
      const float RADIUS, 
      const float BIAS){

    vec3 tangent = normalize(randomDir - vNormal * dot(randomDir, vNormal));
    vec3 bitangent = cross(vNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, vNormal);

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i].xyz; // from tangent to view-space
        samplePos = vPos + samplePos * RADIUS; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = camera.proj * offset; 
        offset.xyz /= offset.w; 
        offset.xyz = offset.xyz * 0.5 + 0.5; 

        // get sample depth
        // float sampleDepth = texture(depthBuffer, offset.xy).r; // get depth value of kernel sample
        float sampleDepth = texture(vPosBuffer, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(vPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / KERNEL_SIZE);
    return occlusion;
}

// https://www.cs.rpi.edu/~cutler/classes/advancedgraphics/S11/final_projects/chamberlin_sullivan.pdf
// float UnsharpSSAO(sampler2D ssaoMap){

//     const float kernelDimension = 15.0;
//     const ivec2 screenSize = textureSize(ssaoMap,0);

//     float occlusion = 0.0;

//     int i = int(gl_FragCoord.x);
//     int j = int(gl_FragCoord.y);

//     int maxX = i + int(floor(kernelDimension*0.5));
//     int maxY = j + int(floor(kernelDimension*0.5));

//     float sampX;
//     float sampY;

//     float neighborCount = 0;

//     for (int x = i - int(floor(kernelDimension*0.5)); x < maxX; x++) {
//     for (int y = j - int(floor(kernelDimension*0.5)); y < maxY; y++) {
    
//     sampX = float(x) / screenSize.x;
//     sampY = float(y) / screenSize.y;

//     if (sampX >= 0.0 && sampX <= 1.0 && sampY >= 0.0 && sampY <= 1.0 &&
    
//     abs( linearizeDepth(texture(ssaoMap,gl_FragCoord.xy / screenSize.xy).a,0.1,100.0) -
//      linearizeDepth(texture(ssaoMap,vec2(sampX,sampY)).a, 0.1,100.0)) < 0.02) {
//     occlusion +=   linearizeDepth(texture(ssaoMap,vec2(sampX,sampY)).a,0.1,100.0);
//     neighborCount++;
//     }
//     }
//     }

//     occlusion = occlusion / neighborCount;
     
     
//     occlusion = 20 * ( linearizeDepth(texture(ssaoMap,gl_FragCoord.xy / screenSize.xy).a, 0.1,100.0) - max(0.0, occlusion));


//   return occlusion;

// }

