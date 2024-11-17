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


layout(lines) in;
layout(line_strip, max_vertices = 100) out;


void main() {
    for(int i = 0; i < MAX_LIGHTS; i++) {

        //Stop emitting vertex if theres no more lights
        if(i>=scene.numLights) break;

        gl_Layer = i;
        if(scene.lights[i].type == 2) continue; //If some light is raytraced

        gl_Position = scene.lights[i].viewProj * object.model*gl_in[0].gl_Position;
        EmitVertex();
        
        gl_Position = scene.lights[i].viewProj * object.model*gl_in[1].gl_Position;
        EmitVertex();
        

        EndPrimitive();
        
      
    }
}


#shader fragment
#version 460
#include camera.glsl
#include utils.glsl

layout(location = 0) out vec2 outVSM;

void main() {

    outVSM = vec2(gl_FragCoord.z, gl_FragCoord.z * gl_FragCoord.z);
}
