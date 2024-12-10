#shader vertex
#version 460
#include camera.glsl

//INPUT
layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

//OUTPUT
layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);

    v_uv = uv;
}

#shader fragment
#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
#include material_defines.glsl
#include camera.glsl
#include light.glsl
#include scene.glsl
#include utils.glsl
#include shadow_mapping.glsl
#include fresnel.glsl
#include ssao.glsl
#include IBL.glsl
#include reindhart.glsl
#include BRDFs/schlick_smith_BRDF.glsl
#include BRDFs/marschner_BSDF.glsl
#include raytracing.glsl

//INPUT
layout(location = 0) in  vec2 v_uv;

//OUTPUT
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBrightColor;

//UNIFORMS
layout(set = 0, binding =   2) uniform sampler2DArray              shadowMap;
layout(set = 0, binding =   4) uniform samplerCube                 irradianceMap;
layout(set = 0,  binding =  5) uniform accelerationStructureEXT    TLAS;
layout(set = 0,  binding =  6) uniform sampler2D                   blueNoiseMap;
//G-BUFFER
layout(set = 1, binding = 0) uniform sampler2D positionBuffer;
layout(set = 1, binding = 1) uniform sampler2D normalBuffer;
layout(set = 1, binding = 2) uniform sampler2D colorBuffer;
layout(set = 1, binding = 3) uniform sampler2D materialBuffer;
layout(set = 1, binding = 4) uniform sampler2D emissionBuffer;
// layout(set = 1, binding = 5) uniform sampler2D tempBuffer;

//SURFACE PROPERTIES
vec3    g_pos; 
float   g_depth; 
vec3    g_normal; 
vec3    g_albedo; 
float   g_opacity;
vec4    g_material; 
vec3    g_emission; 
vec4    g_temp; 

void main()
{

    //////////////////////////////////////
    // SETUP SURFACE
    //////////////////////////////////////
    vec4 positionData = texture(positionBuffer,v_uv);
    g_pos       = positionData.rgb;
    g_depth     = positionData.w;
    g_normal    = normalize(texture(normalBuffer,v_uv).rgb);
    vec4 colorData = texture(colorBuffer,v_uv);
    g_albedo    = colorData.rgb;
    g_opacity   = colorData.w;
    g_material  = texture(materialBuffer,v_uv);
    g_emission  = texture(emissionBuffer,v_uv).rgb;
    g_temp      = vec4(0.0);

    vec3 color = vec3(0.0);
    //////////////////////////////////////
    // IF LIT MATERIAL
    //////////////////////////////////////
    if(g_material.w != UNLIT_MATERIAL){

        vec3 direct = vec3(0.0);
        vec3 ambient = vec3(0.0);
        vec3 modelPos = (camera.invView * vec4(g_pos.xyz, 1.0)).xyz;
        vec3 modelNormal = (camera.invView  * vec4(g_normal.xyz, 0.0)).xyz;
        //////////////////////////////////////
        // PHYSICAL 
        //////////////////////////////////////
        if(g_material.w == PHYSICAL_MATERIAL){
            //Populate BRDF ________________________
            SchlickSmithBRDF brdf;
            brdf.albedo = g_albedo;
            brdf.opacity = g_opacity;
            brdf.normal = g_normal;
            brdf.roughness = g_material.x;
            brdf.metalness = g_material.y;
            brdf.ao = g_material.z;
            // brdf.emission;
            brdf.F0 = vec3(0.04);
            brdf.F0 = mix(brdf.F0, brdf.albedo, brdf.metalness);
            brdf.emission = g_emission;

            for(int i = 0; i < scene.numLights; i++) {
                    //If inside liught area influence
                    if(isInAreaOfInfluence(scene.lights[i], g_pos)){
                        //Direct Component ________________________
                        vec3 lighting = vec3(0.0);
                        lighting = evalSchlickSmithBRDF( 
                            scene.lights[i].type != DIRECTIONAL_LIGHT ? normalize(scene.lights[i].position - g_pos) : normalize(scene.lights[i].position.xyz), //wi
                            normalize(-g_pos),                                                                                           //wo
                            scene.lights[i].color * computeAttenuation( scene.lights[i], g_pos) *  scene.lights[i].intensity,              //radiance
                            brdf
                            );
                        //Shadow Component ________________________
                        if(scene.lights[i].shadowCast == 1) {
                            if(scene.lights[i].shadowType == 0) //Classic
                                lighting *= computeShadow(shadowMap, scene.lights[i], i, modelPos);
                            if(scene.lights[i].shadowType == 1) //VSM   
                                lighting *= computeVarianceShadow(shadowMap, scene.lights[i], i, modelPos);
                            if(scene.lights[i].shadowType == 2) //Raytraced  
                                lighting *= computeRaytracedShadow(
                                    TLAS, 
                                    blueNoiseMap,
                                    modelPos, 
                                    scene.lights[i].type != DIRECTIONAL_LIGHT ? scene.lights[i].shadowData.xyz - modelPos : scene.lights[i].shadowData.xyz,
                                    int(scene.lights[i].shadowData.w), 
                                    scene.lights[i].area, 
                                    0);
                        }
                    direct += lighting;
                    }
            }
            //Emission Component ________________________
            direct += brdf.emission;
            //Ambient Component ________________________
            if(scene.useIBL){
                ambient = computeAmbient(
                    irradianceMap,
                    scene.envRotation,
                    modelNormal,
                    normalize(camera.position.xyz-modelPos),
                    brdf.albedo,
                    brdf.F0,
                    brdf.metalness,
                    brdf.roughness,
                    scene.ambientIntensity);
            }else{
                ambient = (scene.ambientIntensity * scene.ambientColor) * brdf.albedo;
            }
            ambient *= brdf.ao;
        }
        //////////////////////////////////////
        // PHONG 
        //////////////////////////////////////
        if(g_material.w == PHONG_MATERIAL){
        
            //TBD ....
        }
        //////////////////////////////////////
        // HAIR 
        //////////////////////////////////////
        if(g_material.w == HAIR_STRAND_MATERIAL){
            //TBD ....
        }

               
                    

    //color = reindhartTonemap(direct + ambient);
    color = direct + ambient;
            
    //////////////////////////////////////
    // IF UNLIT MATERIAL
    //////////////////////////////////////
    }else{
        color = g_albedo;
    }
    

    if(scene.enableFog){
        float f = computeFog(g_depth);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }


    outColor = vec4(color,1.0);

    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        outBrightColor = vec4(color, 1.0);
    else
        outBrightColor = vec4(0.0, 0.0, 0.0, 1.0);

   
    

}

