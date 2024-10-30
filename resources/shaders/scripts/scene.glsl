#define MAX_LIGHTS 50

layout(set = 0, binding = 1) uniform SceneUniforms {
    vec3 fogColor;

    bool enableSSAO;

    float fogMinDistance;
    float fogMaxDistance;
    float fogIntensity;

    bool enableFog;

    vec3 ambientColor;
    float ambientIntensity;

    LightUniform lights[MAX_LIGHTS];
    int numLights;

    int SSAOType;
    bool emphasizeAO;
    
    bool useIBL;
    float envRotation;
    float envColorMultiplier;
} scene;

float computeFog(float coordDepth) {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - coordDepth * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * 0.01 * z);
}