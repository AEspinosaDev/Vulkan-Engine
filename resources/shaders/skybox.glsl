#shader vertex
#version 460 
#include camera.glsl

layout(location = 0) in vec3 position;


layout(location = 0) out vec3 _uv;


void main()
{
    _uv  = position;
    vec4 outPos =  camera.proj * mat4(mat3(camera.view)) * vec4(position, 1.0); //Take out translation of the equation
    gl_Position = outPos.xyww;
}  

#shader fragment
#version 460 

layout(location = 0) in vec3 _uv;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 3) uniform samplerCube u_skymap;

void main()
{    
    vec3 color =texture(u_skymap, _uv).rgb;
    fragColor = vec4(color,1.0);
}
