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
        return 1.0;
    
    return 1.0 - filterPCF(shadowMap, lightId,int(light.shadowData.w), light.shadowData.y, projCoords, light.shadowData.x);

}


//Control light leaking
float linstep(float low, float high, float v){
    return clamp((v-low)/(high-low),0.0,1.0);
}

float computeVarianceShadow(sampler2DArray VSM ,LightUniform light, int lightId, vec3 fragModelPos) {

    vec4 pos_lightSpace = light.viewProj * vec4(fragModelPos, 1.0);

    vec3 projCoords = pos_lightSpace.xyz / pos_lightSpace.w;

    projCoords.xy  = projCoords.xy * 0.5 + 0.5;

    // ChebyshevUpperBound {
    vec2 moments = texture(VSM, vec3(projCoords.xy,lightId)).rg;
    float p = step(projCoords.z,moments.x);
    float variance = max(moments.y-moments.x*moments.x,0.00002);

    float d = projCoords.z - moments.x;
    float pMax = linstep(light.shadowData.x,1.0,variance / (variance + d*d));
    //}

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 1.0;

    return min(max(p,pMax),1.0);

}





