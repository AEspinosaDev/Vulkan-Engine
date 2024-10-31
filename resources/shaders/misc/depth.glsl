#shader vertex
#version 460

layout(location = 0) in vec3 pos;

layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
} object;

void main() {
    gl_Position = camera.viewProj * object.model * vec4(pos, 1.0);
}

#shader fragment
#version 460


void main() {
}
