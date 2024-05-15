#shader vertex
#version 460


layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);

    v_uv = uv;
}

#shader fragment
#version 460
#define MAX_LIGHTS 50

layout(location = 0) in  vec2 v_uv;


struct LightUniform{
   vec3 position;
    int type;
    vec3 color;
    float intensity;
    vec4 data;

    mat4 viewProj;

    float shadowBias;
    bool apiBiasEnabled;
    bool angleDependantBias;
    float pcfKernel;
};

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
} scene;


layout(set = 3, binding = 0) uniform sampler2D positionBuffer;
layout(set = 3, binding = 1) uniform sampler2D normalBuffer;
layout(set = 3, binding = 2) uniform sampler2D albedoBuffer;
layout(set = 3, binding = 3) uniform sampler2D materialBuffer;


layout(location = 0) out vec4 outColor;


void main()
{
   

   outColor = vec4(texture(positionBuffer,v_uv).rgb,1.0);
}

