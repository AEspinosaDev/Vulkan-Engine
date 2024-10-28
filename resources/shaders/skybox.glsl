#shader vertex
#version 460 
#include camera.glsl

layout(location = 0) in vec3 position;


layout(location = 0) out vec3 _uv;

layout(push_constant) uniform SkyboxConstants {
    float blurriness;
    float intensity;
    float rotation;
} skybox;

mat4 rotationY(float angle) {
    float radAngle = radians(angle);
    float c = cos(angle);
    float s = sin(angle);
    
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
    vec4 outPos =  camera.proj * mat4(mat3(camera.view)) * rotationY(skybox.rotation) * vec4(position, 1.0); //Take out translation of the equation
    gl_Position = outPos.xyww;
}  

#shader fragment
#version 460 

layout(location = 0) in vec3 _uv;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform SkyboxConstants {
    float blurriness;
    float intensity;
    float rotation;
} skybox;

layout(set = 0, binding = 3) uniform samplerCube u_skymap;

void main()
{    
    vec3 color =texture(u_skymap, _uv).rgb * skybox.intensity;
    fragColor = vec4(color,1.0);
}
