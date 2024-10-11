float filterPCF(sampler2DArray shadowMap ,int lightId ,int kernelSize, float extentMultiplier, vec3 coords, float bias) {

    int edge = kernelSize / 2;
    vec3 texelSize = 1.0 / textureSize(shadowMap, 0);

    float currentDepth = coords.z;

    float shadow = 0.0;

    for(int x = -edge; x <= edge; ++x) {
        for(int y = -edge; y <= edge; ++y) {
            float pcfDepth = texture(shadowMap, vec3(coords.xy + vec2(x, y) * texelSize.xy * extentMultiplier,lightId)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow /= (kernelSize * kernelSize);

}

float computeShadow(sampler2DArray shadowMap ,LightUniform light, int lightId, vec3 fragModelPos) {

    vec4 pos_lightSpace = light.viewProj * vec4(fragModelPos, 1.0);

    vec3 projCoords = pos_lightSpace.xyz / pos_lightSpace.w;

    projCoords.xy  = projCoords.xy * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    
    return filterPCF(shadowMap, lightId,int(light.pcfKernel), light.kernelRadius, projCoords, light.shadowBias);

}