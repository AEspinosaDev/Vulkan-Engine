layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4    view;
    mat4    proj;
    mat4    viewProj;
    /*Inversed*/
    mat4    invView;
    mat4    invProj;
    mat4    invViewProj;
    /*Other data*/    
    vec4    position;
    vec2    screenExtent;
    float   nearPlane;
    float   farPlane;
} camera;
