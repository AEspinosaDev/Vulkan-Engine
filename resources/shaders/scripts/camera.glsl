layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec4 position;
    vec2 screenExtent;
} camera;
