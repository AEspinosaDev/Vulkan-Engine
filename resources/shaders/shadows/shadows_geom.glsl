#shader vertex
#version 460

//Input VBO
layout(location = 0) in vec3 pos;

void main() {
   gl_Position = vec4(pos, 1.0);
}

#shader geometry
#version 460
#include light.glsl
#include scene.glsl
#include object.glsl

layout(triangles) in;
layout(triangle_strip, max_vertices = 150) out;


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
        
        gl_Position = scene.lights[i].viewProj * object.model*gl_in[2].gl_Position;
        EmitVertex();

        EndPrimitive();
        
      
    }
}


#shader fragment
#version 460

void main() {
}
