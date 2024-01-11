#shader vertex
#version 460

//Input VBO
layout(location = 0) in vec3 pos;

//Uniforms
layout(set = 0, binding = 1) uniform SceneUniforms {
    vec3 fogColor;
    float fogExponent;
    float fogMinDistance;
    float fogMaxDistance;
    float fogIntensity;

    float unusedSlot1;

    vec3 ambientColor;
    float ambientIntensity;

    vec3 lighPosition;
    float lightType;
    vec3 lightColor;
    float lightIntensity;
    vec4 lightData;
    mat4 lightViewProj;

} scene;
layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
} object;

void main() {
    gl_Position = scene.lightViewProj * object.model * vec4(pos, 1.0);
}

#shader fragment
#version 460

layout(location = 0) out vec4 color;

void main() {
    color = vec4(1.0, 0.0, 0.0, 1.0);
}