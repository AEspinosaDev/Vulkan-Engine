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

layout(location = 0) in  vec2 v_uv;

layout(set = 0, binding = 0) uniform sampler2D ssaoBuffer;

layout(location = 0) out vec4 outOcclusion;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoBuffer, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoBuffer, v_uv + offset).r;
        }
    }
    float occlusion = result / (4.0f*4.0f);
     outOcclusion = vec4(vec3(occlusion),1.0);
}

