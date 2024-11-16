layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4    model;
    vec4    otherParams;
    
    int     selected;
    vec3    volumeCenter;
} object;