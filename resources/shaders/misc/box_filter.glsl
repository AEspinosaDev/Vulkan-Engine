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

layout(set = 1, binding = 0) uniform sampler2D inputImage;



layout(location = 0) out vec4 outputImage;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(inputImage, 0));
    float result = 0.0;
    for (int x = -4; x < 4; ++x) 
    {
        for (int y = -4; y < 4; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(inputImage, v_uv + offset).r;
        }
    }
    result = result / (8.0f*8.0f);
    outputImage = vec4(result);
}

