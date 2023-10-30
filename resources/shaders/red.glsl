#shader vertex
#version 450

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);


void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}


#shader fragment
#version 450


layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0,0.0,0.0, 1.0);
}