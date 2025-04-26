#shader vertex
#version 460 
#include camera.glsl
#include light.glsl
#include scene.glsl

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 _uv;

mat4 rotationY(float angle) {
    float radAngle = radians(angle);
    float c = cos(radAngle);
    float s = sin(radAngle);
    
    return mat4(
        c,  0.0, s,  0.0,
        0.0, 1.0, 0.0, 0.0,
        -s, 0.0, c,  0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

void main()
{
    _uv  = position;
    vec4 outPos =  camera.proj * mat4(mat3(camera.view)) * rotationY(scene.envRotation) * vec4(position, 1.0); //Take out translation of the equation
    gl_Position = outPos.xyww;
}  

#shader fragment
#version 460 
#include light.glsl
#include scene.glsl
#include material_defines.glsl

layout(location = 0) in vec3 _uv;

//Output
layout(location = 0) out vec4 outNormal;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outMaterial;
layout(location = 3) out vec4 outTemporal;

//Uniforms
layout(set = 0, binding = 3) uniform samplerCube envMap;

void main()
{    
    vec3 color =texture(envMap, _uv).rgb * scene.envColorMultiplier;
    
    outNormal   = vec4(0.0);
    outAlbedo   = vec4(color,1.0);
    outMaterial = vec4(0.0); //w material ID
    outMaterial.w = UNLIT_MATERIAL; //w material ID
    outTemporal = vec4(0.0); //TBD
}
