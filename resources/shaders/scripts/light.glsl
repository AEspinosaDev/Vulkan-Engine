struct LightUniform{

    vec3    position;
    int     type;
    vec3    color;
    float   intensity;

    float   areaEffect;
    float   decay;

    //Shadows
    float   shadowType;
    float   shadowCast;
    mat4    viewProj;
    float   shadowBias;
    float   kernelRadius;
    bool    angleDependantBias;
    float   pcfKernel;

};

bool isInAreaOfInfluence(LightUniform light, vec3 fragPos){
    if(light.type == 0){ //Point Light
        return length(light.position - fragPos) <= light.areaEffect;
    }
    else if(light.type == 2){ //Spot light
        //TO DO...
        return true;
    }
    return true; //Directional influence is total
}

float computeAttenuation(LightUniform light, vec3 fragPos) {
    if(light.type != 0)
        return 1.0;

    float d = length(light.position - fragPos);
    float influence = light.areaEffect;
    float window = pow(max(1 - pow(d / influence, 2), 0), 2);

    return pow(10 / max(d, 0.0001), 2) * window;
}