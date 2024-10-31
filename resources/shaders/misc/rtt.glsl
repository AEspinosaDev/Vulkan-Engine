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

layout(set = 0, binding = 0) uniform sampler2D inColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(texture(inColor,v_uv).rgb,1.0);
}

