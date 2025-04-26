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
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
#include camera.glsl
#include light.glsl
#include scene.glsl
#include warp.glsl
#include SSAO.glsl
#include raytracing.glsl

layout(location = 0) in vec2 v_uv;

//G - BUFFER
layout(set = 0, binding = 2) uniform sampler2D depthBuffer;
layout(set = 0, binding = 3) uniform sampler2D normalBuffer;
//SSAO
layout(set = 0, binding = 4) uniform SampleKernel {
    vec4 samples[MAX_SSAO_SAMPLES];
} kernel;
//RTX
layout(set = 0, binding = 5) uniform sampler2D blueNoiseBuffer;
layout(set = 0, binding = 6) uniform accelerationStructureEXT TLAS;

//SSAO SETTINGS
layout(push_constant) uniform Settings {
    float radius;
    float bias;
    int samples;
    uint type;
    uint enabled;
} ssao;

layout(location = 0) out vec2 outColor; //x = Occlusion; y = RT Shadows

//SURFACE PROPERTIES
vec3 g_pos;
float g_depth;
vec3 g_normal;

void main() {
   // Build position from depth buffer
    g_depth = texture(depthBuffer, v_uv).r;
    vec2 ndc = v_uv * 2.0 - 1.0;
    vec4 clip = vec4(ndc, g_depth, 1.0);
    vec4 viewPos = camera.invProj * clip;
    viewPos /= viewPos.w;
    g_pos = viewPos.xyz;

    g_normal = normalize(texture(normalBuffer, v_uv).rgb);

    //////////////////////////////////////////////////
    // AMBIENT OCCLUSION
    //////////////////////////////////////////////////
    vec3 randomVec = normalize(texture(blueNoiseBuffer, v_uv).xyz) * 2.0 - 1.0;

    float occlusion = 1.0;
    if(ssao.enabled == 1) {
        if(ssao.type == 0) //SSAO
            occlusion = SSAO(depthBuffer, v_uv, g_pos, g_normal, randomVec, kernel.samples, ssao.samples, ssao.radius, ssao.bias);
        if(ssao.type == 1) { //Raytraced AO
            vec3 modelPos = (camera.invView * vec4(g_pos.xyz, 1.0)).xyz;
            vec3 modelNormal = (camera.invView * vec4(g_normal.xyz, 0.0)).xyz;
            occlusion = computeRaytracedAO(TLAS, blueNoiseBuffer, modelPos, modelNormal, ssao.samples, EPSILON, ssao.radius, 0);
        }
    }

    //////////////////////////////////////////////////
    // RAYTRACED SHADOWS
    //////////////////////////////////////////////////
    // vec3 modelPos = (camera.invView * vec4(g_pos.xyz, 1.0)).xyz;
    // float shadow = 0.0;
    // for(int i = 0; i < scene.numLights; i++) {
    //     if(isInAreaOfInfluence(scene.lights[i], g_pos)){
    //         if(scene.lights[i].shadowType == 2) //If Raytraced  
    //             shadow += computeRaytracedShadow(
    //                 TLAS, 
    //                 blueNoiseBuffer,
    //                 modelPos, 
    //                 scene.lights[i].type != DIRECTIONAL_LIGHT ? scene.lights[i].shadowData.xyz - modelPos : scene.lights[i].shadowData.xyz,
    //                 int(scene.lights[i].shadowData.w), 
    //                 scene.lights[i].area, 
    //                 0);
    //     }

    // }

    outColor = vec2(occlusion, 0.0);
}
