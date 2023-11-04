#shader vertex
#version 450
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in vec3 color;

void main() {
    gl_Position = vec4(pos, 1.0);
}


#shader fragment
#version 450


layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0,0.0,0.0, 1.0);
}