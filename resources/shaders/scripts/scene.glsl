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
} scene;

float computeFog() {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - gl_FragCoord.z * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * 0.01 * z);
}