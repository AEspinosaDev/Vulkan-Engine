#shader vertex
#version 460

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);
}

#shader fragment
#version 460

layout(location = 0) in  vec2 v_uv;

//Output
// layout(location = 0) out float outOcclusion;
//Output
layout(location = 0) out vec4 outOcclusion;

void main() {
    // vec3 normal = v_normal * 0.5 + 0.5;

   outOcclusion = vec4(1.0f,0.0f,0.0f,1.0f);
}
