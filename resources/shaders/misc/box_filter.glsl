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

/*Modify values of set and binding according to needs*/
layout(set = 1, binding = 0) uniform sampler2D inputImage;

layout(push_constant) uniform Settings {
    float radius; 
} settings;


layout(location = 0) out vec4 outputImage;

void main()
{
    int radius = int(settings.radius);
    vec2 texelSize = 1.0 / vec2(textureSize(inputImage, 0));
    float result = 0.0;
    for (int x = -radius; x < radius; ++x) 
    {
        for (int y = -radius; y < radius; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(inputImage, v_uv + offset).r;
        }
    }
    result = result / ((radius*2.0f)*(radius*2.0f));
    outputImage = vec4(result);
}

