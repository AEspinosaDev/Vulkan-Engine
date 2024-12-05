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

layout(set = 0, binding = 2) uniform sampler2D srcImage;    //F32
layout(set = 0, binding = 3) uniform sampler2D bloomImage;  //F32

layout(location = 0) out vec4 outputImage;


void main()
{
    vec3 hdrColor = texture(srcImage, v_uv).rgb;
    vec3 bloomColor = texture(bloomImage, v_uv).rgb;
    outputImage =  vec4(mix(hdrColor, bloomColor, 0.05),1.0); 
    // outputImage =  vec4(hdrColor,1.0); 
}