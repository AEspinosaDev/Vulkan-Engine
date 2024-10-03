#shader vertex
#version 460 core

//Input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 color;

//Uniforms
layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 otherParams;
    int selected;
} object;

//Output
layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_tangent;

void main() {

    gl_Position = object.model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(object.model))) * tangent);
    v_color = color;

}

#shader geometry
#version 460 core

//Setup
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

//Input
layout(location = 0) in vec3 v_color[];
layout(location = 1) in vec3 v_tangent[];

//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec4 position;
     vec2 screenExtent;
} camera;

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;
} material;

//Output
layout(location = 0) out vec3 g_pos;
layout(location = 1) out vec2 g_uv;
layout(location = 2) out vec3 g_dir;
layout(location = 3) out vec3 g_color;

void emitQuadPoint(
    vec4 origin,
    vec4 right,
    float offset,
    vec2 uv,
    int id
) {

    vec4 newPos = origin + right * offset; //Model space
    gl_Position = camera.viewProj * newPos;
    g_dir = normalize(mat3(transpose(inverse(camera.view))) * v_tangent[id]);
    g_color = v_color[id];
    g_pos = (camera.view * newPos).xyz;
    g_uv = uv;

    EmitVertex();
}

void main() {

        //Model space --->>>

    vec4 startPoint = gl_in[0].gl_Position;
    vec4 endPoint = gl_in[1].gl_Position;

    vec4 view0 = vec4(camera.position.xyz, 1.0) - startPoint;
    vec4 view1 = vec4(camera.position.xyz, 1.0) - endPoint;

    vec3 dir0 = v_tangent[0];
    vec3 dir1 = v_tangent[1];

    vec4 right0 = normalize(vec4(cross(dir0.xyz, view0.xyz), 0.0));
    vec4 right1 = normalize(vec4(cross(dir1.xyz, view1.xyz), 0.0));

        //<<<----

    float halfLength = material.thickness * 0.5;

    emitQuadPoint(startPoint, right0, halfLength, vec2(1.0, 0.0), 0);
    emitQuadPoint(endPoint, right1, halfLength, vec2(1.0, 1.0), 1);
    emitQuadPoint(startPoint, -right0, halfLength, vec2(0.0, 0.0), 0);
    emitQuadPoint(endPoint, -right1, halfLength, vec2(0.0, 1.0), 1);

}

#shader fragment
#version 460 core

layout(location = 0) in vec3 g_pos;
layout(location = 1) in vec2 g_uv;
layout(location = 2) in vec3 g_dir;
layout(location = 3) in vec3 g_color;



//Output
layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outDir;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaterial;


void main() {


    outPos = vec4(g_pos,gl_FragCoord.z);
    outDir = vec4( g_dir , 1.0f );
    outAlbedo = vec4(g_color,1.0);
    outMaterial = vec4(0.0,0.0,0.0, 1.0); //w material id

}