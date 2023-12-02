#shader vertex
#version 460

//Input VBO
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in vec3 color;

//Output
layout(location = 0) out vec3 fragColor;

//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} camera;
layout(set = 1, binding = 0) uniform ObjectUniforms{
	mat4 model;
} object;

void main() {
    gl_Position = camera.viewProj*object.model * mat4(1.0f) * vec4(pos,1.0f);
    fragColor = color;
}


#shader fragment
#version 460


layout(location = 0) in vec3 fragColor;

layout(set = 0, binding = 1) uniform SceneUniforms{
    vec4 fogColor; // w is for exponent
	vec4 fogDistances; //x for min, y for max, zw unused.
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} scene;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(scene.ambientColor.rgb, 1.0);
}