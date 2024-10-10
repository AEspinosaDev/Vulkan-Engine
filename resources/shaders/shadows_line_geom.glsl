#shader vertex
#version 460

//Input VBO
layout(location = 0) in vec3 pos;

void main() {
   gl_Position = vec4(pos, 1.0);
}

#shader geometry
#version 460

#define MAX_LIGHTS 50

layout(lines) in;
layout(line_strip, max_vertices = 100) out;

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
    LightUniform lights[MAX_LIGHTS];
    int numLights;
} scene;


layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
} object;

void main() {
    for(int i = 0; i < MAX_LIGHTS; i++) {

        //Stop emitting vertex if theres no more lights
        if(i>=scene.numLights) break;

        //Check if object inside area of light

        gl_Layer = i;

        gl_Position = scene.lights[i].viewProj * object.model*gl_in[0].gl_Position;
        EmitVertex();
        
        gl_Position = scene.lights[i].viewProj * object.model*gl_in[1].gl_Position;
        EmitVertex();
        

        EndPrimitive();
        
      
    }
}


#shader fragment
#version 460

void main() {
}
