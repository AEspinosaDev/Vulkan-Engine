layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4    view;
    mat4    proj;
    mat4    viewProj;
    /*Inversed*/
    mat4    invView;
    mat4    invProj;
    mat4    invViewProj;
    /*Aux mats*/
    mat4    unormProj;
    /*Other data*/    
    vec4    position;
    vec2    screenExtent;
    float   nearPlane;
    float   farPlane;
} camera;

// Linearise non-linear depth value into range [near..far]
float lineariseDepth(float depth) {
    float near = camera.nearPlane;
    float far = camera.farPlane;
    return (near * far) / (far - depth * (far - near));
}

//  const glm::mat4 W {
//             width / 2.0f, 0.0f, 0.0f, 0.0f,
//             0.0f, height / 2.0f, 0.0f, 0.0f,
//             0.0f, 0.0f, 1.0f, 0.0f,
//             width / 2.0f, height / 2.0f, 0.0f, 1.0f,
//         };