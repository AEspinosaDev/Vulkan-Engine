#define POINT_LIGHT         0
#define DIRECTIONAL_LIGHT   1
#define SPOT_LIGHT          2

struct LightUniform{

    vec3    position;
    float   type;

    vec3    color;
    float   intensity;

    float   areaEffect;
    float   area;          //Area if raytraced
    float   shadowType;
    float   shadowCast;

    mat4    viewProj;

    vec4    shadowData;

    vec3    direction;
    float   data;

    // x   shadowBias;
    // y   kernelRadius;
    // z   angleDependantBias;
    // w   pcfKernel;

    //If raytraced
    // vec3 world position
    // w samples

};

bool isInAreaOfInfluence(LightUniform light, vec3 fragPos){
    if(int(light.type) == 0){ //Point Light
        return length(light.position - fragPos) <= light.areaEffect;
    }
    else if(int(light.type) == 2){ //Spot light
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