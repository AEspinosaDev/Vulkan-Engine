#shader vertex
#version 450

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

void main() {
    gl_Position = camera.viewProj * mat4(1.0f) * vec4(pos,1.0f);
    fragColor = color;
}


#shader fragment
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}