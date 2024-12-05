#shader vertex
#version 460

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;


layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);
    v_uv = uv;
}

#shader fragment
#version 460
#include reindhart.glsl

layout(location = 0) in  vec2 v_uv;

layout(set = 0, binding = 0) uniform sampler2D inputImage;

layout(location = 0) out vec4 outputImage;



void main()
{
    vec3 result = texture(inputImage,v_uv).rgb;
    result = vec3(1.0) - exp(-result * 1.0);

    // // also gamma correct while we're at it
    // const float GAMMA = 2.2;
    // result = pow(result, vec3(1.0 / GAMMA));

    outputImage = vec4(result,1.0); //WIP
}