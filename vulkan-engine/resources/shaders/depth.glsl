#shader vertex
#version 460

#define MAX_LIGHTS 10

//Input VBO
layout(location = 0) in vec3 pos;

struct LightUniform{
   vec3 position;
    int type;
    vec3 color;
    float intensity;
    vec4 data;

    mat4 viewProj;

    float shadowBias;
    bool apiBiasEnabled;
    bool angleDependantBias;
    float pcfKernel;
};

//Uniforms
layout(set = 0, binding = 1) uniform SceneUniforms {
    vec4 unusedSlot1;
    vec4 unusedSlot2;
    vec4 unusedSlot3;
    LightUniform lights[10];
    int numLights;
} scene;

layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
} object;

// layout(location = 0) out VS_OUT{
// 	mat4 modelViews[2];
// } vs_out;

void main() {
    //gl_Position = scene.lights[0].viewProj * object.model * vec4(pos, 1.0);
    // for(int i = 0; i < 2; ++i) {
    //     vs_out.modelViews[i] = scene.lights[i].viewProj*object.model;
    // }
   gl_Position = vec4(pos, 1.0);
}

#shader geometry
#version 460
// #extension GL_EXT_geometry_shader : require

layout(triangles) in;
layout(triangle_strip, max_vertices = 20) out;

struct LightUniform{
   vec3 position;
    int type;
    vec3 color;
    float intensity;
    vec4 data;

    mat4 viewProj;

    float shadowBias;
    bool apiBiasEnabled;
    bool angleDependantBias;
    float pcfKernel;
};

layout(set = 0, binding = 1) uniform SceneUniforms {
    vec4 unusedSlot1;
    vec4 unusedSlot2;
    vec4 unusedSlot3;
    LightUniform lights[10];
    int numLights;
} scene;


layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
} object;

// layout(location = 0) in VS_OUT{
// 	mat4 modelViews[2];
// } gs_in[];

void main() {
    for(int i = 0; i < 10; i++) {
        gl_Layer = i;

        gl_Position = scene.lights[i].viewProj * object.model*gl_in[0].gl_Position;
        EmitVertex();
        
        gl_Position = scene.lights[i].viewProj * object.model*gl_in[1].gl_Position;
        EmitVertex();
        
        gl_Position = scene.lights[i].viewProj * object.model*gl_in[2].gl_Position;
        EmitVertex();
    EndPrimitive();
      
    }
}


#shader fragment
#version 460

layout(location = 0) out vec4 color;

void main() {
    color = vec4(1.0, 0.0, 0.0, 1.0);
    // gl_FragDepth = 1.0;
}
