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

layout(location = 0) in vec3 _uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 outBrightColor;

layout(set = 0, binding = 3) uniform samplerCube envMap;

void main()
{    
    vec3 color =texture(envMap, _uv).rgb * scene.envColorMultiplier;
    
    if(scene.enableFog) {
        float f = computeFog(gl_FragCoord.z);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    fragColor = vec4(color,1.0);

    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        outBrightColor = vec4(color, 1.0);
    else
        outBrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
