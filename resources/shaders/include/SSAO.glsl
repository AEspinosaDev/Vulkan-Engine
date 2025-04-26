#ifndef MAX_SSAO_SAMPLES
#define MAX_SSAO_SAMPLES 64
#endif
// Classic SSAO
float SSAO(
    sampler2D depthBuffer,
    vec2 uv,
    vec3 vPos,
    vec3 vNormal,
    vec3 randomDir,
    vec4 samples[MAX_SSAO_SAMPLES],
    const int KERNEL_SIZE,
    const float RADIUS,
    const float BIAS
) {

    vec3 tangent = normalize(randomDir - vNormal * dot(randomDir, vNormal));
    vec3 bitangent = cross(vNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, vNormal);

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i) {
        // get sample position
        vec3 samplePos = TBN * samples[i].xyz; // from tangent to view-space
        samplePos = vPos + samplePos * RADIUS; 

        // project sample position to texture coordinates
        vec4 offset = camera.proj * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        if(offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0)
            continue; // outside screen

        float sampleDepth = texture(depthBuffer, offset.xy).r;
        vec2 ndc = uv * 2.0 - 1.0;
        vec4 clip = vec4(ndc, sampleDepth, 1.0);
        vec4 viewPos = camera.invProj * clip;
        viewPos /= viewPos.w;
        vec3 sampleFragPos = viewPos.xyz;

        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(vPos.z - sampleFragPos.z));
        occlusion += (sampleFragPos.z >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(KERNEL_SIZE));
    return occlusion;
}

// float SSAO_fast(
//     sampler2D depthTex,
//     vec2 uv,
//     vec4 samples[MAX_SSAO_SAMPLES],
//     int KERNEL_SIZE,
//     float RADIUS,
//     float BIAS
// ) {
//     float centerDepth = texture(depthTex, uv).r;
//     float occlusion = 0.0;

//     for(int i = 0; i < KERNEL_SIZE; ++i) {
//         vec3 sampleVec = samples[i].xyz;

//         // Rotate sample vector using randomDir (optional for better noise)
//         vec2 sampleOffset = vec2(dot(sampleVec, vec3(1, 0, 0)), dot(sampleVec, vec3(0, 1, 0)));
//         sampleOffset = normalize(sampleOffset);

//         vec2 sampleUV = uv + sampleOffset * RADIUS * 1.0 / (float)textureSize();

//         float sampleDepth = texture(depthTex, sampleUV).r;

//         float range = RADIUS;
//         float depthDiff = centerDepth - sampleDepth;

//         float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(depthDiff + 0.0001));
//         occlusion += (depthDiff > BIAS ? 1.0 : 0.0) * rangeCheck;
//     }

//     occlusion = 1.0 - (occlusion / float(KERNEL_SIZE));
//     return occlusion;
// }
