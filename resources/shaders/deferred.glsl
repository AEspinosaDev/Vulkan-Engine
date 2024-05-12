#shader vertex
#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
} object;

layout(location = 0) out vec3 v_normal;

void main() {
    gl_Position = camera.viewProj * object.model * vec4(pos, 1.0);

    v_normal = normalize(mat3(transpose(inverse( object.model))) * normal);
}

#shader fragment
#version 460

layout(location = 0) in vec3 v_normal;

//Output
layout(location = 0) out vec4 outNormal;

void main() {
    vec3 normal = v_normal * 0.5 + 0.5;

    outNormal = vec4(
        1-normal.x,
        normal.y,
        1-normal.z,
        1.0);
}
