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
//Includes
#include material_defines.glsl
#include camera.glsl
#include light.glsl
#include scene.glsl
#include utils.glsl
#include shadows.glsl
#include fresnel.glsl
#include BRDFs/cook_torrance_BRDF.glsl
#include BRDFs/marschner_BSDF.glsl
#include BRDFs/disney_BSSDF_2015.glsl
#include warp.glsl
#include raytracing.glsl
#include hashing.glsl
#include SSR.glsl
#include VXGI.glsl

//INPUT
layout(location = 0) in vec2 v_uv;

//OUTPUT
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBrightColor;

//UNIFORMS
layout(set = 0, binding = 2) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 4) uniform samplerCube irradianceMap;
layout(set = 0, binding = 5) uniform accelerationStructureEXT TLAS;
layout(set = 0, binding = 6) uniform sampler2D samplerMap;
layout(set = 0, binding = 7) uniform sampler2D brdfMap;
layout(set = 0, binding = 8) uniform sampler3D voxelMap;
//G-BUFFER
layout(set = 1, binding = 0) uniform sampler2D depthBuffer;
layout(set = 1, binding = 1) uniform sampler2D normalBuffer;
layout(set = 1, binding = 2) uniform sampler2D colorBuffer;
layout(set = 1, binding = 3) uniform sampler2D materialBuffer;
layout(set = 1, binding = 4) uniform sampler2D velocityEmissionBuffer;
layout(set = 1, binding = 5) uniform sampler2D preCompositionBuffer;

//SETTINGS
layout(push_constant) uniform Settings {
    uint bufferOutput;
    uint enableAO;
    uint AOtype;
    VXGI vxgi;   //Voxel Based GI
    SSR ssr;    //Screen Space Reflections
} settings;

// DEBUGGING QUERIES
#define LIGHTING_MODE   settings.bufferOutput == 0
#define ALBEDO_OUTPUT   settings.bufferOutput == 1
#define NORMALS_OUTPUT  settings.bufferOutput == 2
#define POSITION_OUTPUT settings.bufferOutput == 3
#define MATERIAL_OUTPUT settings.bufferOutput == 4
#define SSAO_OUTPUT     settings.bufferOutput == 5

//SURFACE PROPERTIES
vec3 g_pos;
float g_depth;
vec3 g_normal;
vec3 g_albedo;
float g_opacity;
vec4 g_material;
vec3 g_emission;
int g_isReflective;
vec4 g_temp;

float evalVisibility(int i, vec3 modelPos) {
    if(scene.lights[i].shadowCast == 1) {
        if(scene.lights[i].shadowType == 0) //Classic
            return computeShadow(shadowMap, scene.lights[i], i, modelPos);
        if(scene.lights[i].shadowType == 1) //VSM   
            return computeVarianceShadow(shadowMap, scene.lights[i], i, modelPos);
        if(scene.lights[i].shadowType == 2) //Raytraced  
            return computeRaytracedShadow(TLAS, samplerMap, modelPos, scene.lights[i].type != DIRECTIONAL_LIGHT ? scene.lights[i].worldPosition.xyz - modelPos : -scene.lights[i].shadowData.xyz, int(scene.lights[i].shadowData.w), scene.lights[i].area, scene.lights[i].type != DIRECTIONAL_LIGHT ? length(scene.lights[i].worldPosition.xyz - modelPos) : 30.0, 0);
    }
}

void main() {

    //////////////////////////////////////
    // SETUP SURFACE
    //////////////////////////////////////
    g_depth = texture(depthBuffer, v_uv).r;

    // Build position from depth buffer
    vec2 ndc = v_uv * 2.0 - 1.0;
    vec4 clip = vec4(ndc, g_depth, 1.0);
    vec4 viewPos = camera.invProj * clip;
    viewPos /= viewPos.w;
    g_pos = viewPos.xyz;

    g_normal = normalize(texture(normalBuffer, v_uv).rgb);
    // g_normal = normalize(g_normal);
    vec4 colorData = texture(colorBuffer, v_uv);
    g_albedo = colorData.rgb;
    g_opacity = colorData.w;
    g_material = texture(materialBuffer, v_uv);
    vec4 velocityEmission = texture(velocityEmissionBuffer, v_uv);
    g_emission = g_albedo * velocityEmission.b;
    g_isReflective = int(velocityEmission.w);
    g_temp = vec4(0.0);
    ///////////////////////////////////
    // PRE-COMPUTED DATA
    ///////////////////////////////////
    vec2 preComputedData = texture(preCompositionBuffer, v_uv).rg;
    float SSAO = preComputedData.r;
    float rtShadow = preComputedData.g;
    // float thickness = preComputedData.b;

    vec3 color = vec3(0.0);
    vec3 reflectedColor = vec3(0.0);

    if(LIGHTING_MODE) {
    //////////////////////////////////////
    // IF LIT MATERIAL
    //////////////////////////////////////
        if(g_material.w != UNLIT_MATERIAL) {

            vec3 direct = vec3(0.0);
            vec3 ambient = vec3(0.0);
            vec4 indirect = vec4(0.0);
            vec3 indirectSpecular = vec3(0.0);

            vec3 modelPos = (camera.invView * vec4(g_pos.xyz, 1.0)).xyz;
            vec3 modelNormal = (camera.invView * vec4(g_normal.xyz, 0.0)).xyz;
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // PHYSICAL 
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(g_material.w == PHYSICAL_MATERIAL) {
            //Populate BRDF ________________________
                CookTorranceBRDF brdf;
                brdf.albedo = g_albedo;
                brdf.opacity = g_opacity;
                brdf.normal = g_normal;
                brdf.roughness = g_material.x;
                brdf.metalness = g_material.y;
                brdf.ao = g_material.z;
                brdf.F0 = vec3(0.04);
                brdf.F0 = mix(brdf.F0, brdf.albedo, brdf.metalness);
                brdf.emission = g_emission;

            // Indirect Component ____________________________
                if(settings.vxgi.enabled == 1) {

                // Diffuse
                    indirect = diffuseVoxelGI2(voxelMap, modelPos, modelNormal, settings.vxgi, scene.maxCoord.x - scene.minCoord.x);
                    indirect.rgb = evalDiffuseCookTorranceBRDF(modelNormal, normalize(camera.position.xyz - modelPos), indirect.rgb, brdf);
                    indirect.rgb *= (1.0 - indirect.a); //Account for occlusion

                // Specular
                    vec3 specularConeDirection = reflect(-normalize(camera.position.xyz - modelPos), modelNormal);
                    float voxelWorldSize = (scene.maxCoord.x - scene.minCoord.x) / float(settings.vxgi.resolution);
                    vec3 startPos = modelPos + modelNormal * voxelWorldSize * settings.vxgi.offset;

                    const float CONE_SPREAD = mix(0.005, settings.vxgi.diffuseConeSpread, brdf.roughness);
                    indirectSpecular = evalSpecularCookTorranceBRDF(modelNormal, normalize(camera.position.xyz - modelPos), traceCone(voxelMap, startPos, specularConeDirection, settings.vxgi.maxDistance, CONE_SPREAD, voxelWorldSize).rgb, brdfMap, brdf);

                    indirect.rgb += indirectSpecular.rgb;
                }
            //Direct Component ________________________
                for(int i = 0; i < scene.numLights; i++) {
                    //If inside liught area influence
                    if(isInAreaOfInfluence(scene.lights[i].position, g_pos, scene.lights[i].areaEffect, int(scene.lights[i].type))) {

                        //Direct Component ________________________
                        vec3 lighting = vec3(0.0);
                        lighting = evalCookTorranceBRDF(scene.lights[i].type != DIRECTIONAL_LIGHT ? normalize(scene.lights[i].position - g_pos) : normalize(-scene.lights[i].position.xyz), //wi
                        normalize(-g_pos),                                                                                           //wo
                        scene.lights[i].color * computeAttenuation(scene.lights[i].position, g_pos, scene.lights[i].areaEffect, int(scene.lights[i].type)) * scene.lights[i].intensity,              //radiance
                        brdf);

                        //Visibility Component ________________________
                        lighting *= evalVisibility(i, modelPos);
                        direct += lighting;
                    }
                }
            //Emission Component ________________________
                direct += brdf.emission;
            //Ambient Component ________________________
                if(scene.useIBL) {
                    mat3 rotY = rotationY(radians(scene.envRotation));
                    vec3 rotatedNormal = normalize(rotY * modelNormal);
                    vec3 irradiance = texture(irradianceMap, rotatedNormal).rgb * scene.ambientIntensity;
                    ambient = evalDiffuseCookTorranceBRDF(rotatedNormal, normalize(camera.position.xyz - modelPos), irradiance, brdf);
                } else {
                    ambient = (scene.ambientIntensity * scene.ambientColor) * brdf.albedo;
                }
                ambient *= settings.enableAO == 1 ? settings.AOtype != 2 ? (brdf.ao * SSAO) : 1.0 - indirect.a : brdf.ao;

            //SSR ________________________________
                if(settings.ssr.enabled == 1 && g_isReflective == 1) {
                    vec3 modelPos = vec3(camera.invView * vec4(g_pos, 1.0));
                    vec3 fresnel = fresnelSchlick(max(dot(g_normal, normalize(g_pos)), 0.0), brdf.F0);
                    // reflectedColor = performSSR(settings.ssr, g_pos, g_normal, modelPos, depthBuffer, prevBuffer, brdf.metalness, brdf.roughness, fresnel);
                }

            }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // PHONG 
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(g_material.w == PHONG_MATERIAL) {

            //TBD ....
            }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // HAIR 
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(g_material.w == HAIR_STRAND_MATERIAL) {
                MarschnerBSDF bsdf;
                bsdf.tangent = normalize(g_normal);
                bsdf.baseColor = g_albedo;
                bsdf.beta = g_material.r;
                // bsdf.shift = g_material.g;
                bsdf.shift = 5.2;
                bsdf.ior = 1.55;
                bsdf.Rpower = 2.0;
                bsdf.TTpower = 2.0;
                bsdf.TRTpower = 2.0;


                //Direct Component ________________________
                for(int i = 0; i < scene.numLights; i++) {
                    //If inside liught area influence
                    if(isInAreaOfInfluence(scene.lights[i].position, g_pos, scene.lights[i].areaEffect, int(scene.lights[i].type))) {

                        vec3 lighting = evalMarschnerBSDF(normalize(scene.lights[i].position.xyz - g_pos), normalize(-g_pos), scene.lights[i].color * computeAttenuation(scene.lights[i].position, g_pos, scene.lights[i].areaEffect, int(scene.lights[i].type)) * scene.lights[i].intensity, bsdf,true,true,true);

                        //Visibility Component ________________________
                        lighting *= evalVisibility(i, modelPos);
                        direct += lighting;
                      
                    }
                }
            //Emission Component ________________________
                direct += g_emission;
            //Ambient Component ________________________
                 ambient = (scene.ambientIntensity * scene.ambientColor) * bsdf.baseColor;
            }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // SKIN 
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(g_material.w == SKIN_MATERIAL) {
             
            }

            color = direct + indirect.rgb + ambient + reflectedColor;
    // color = direct + indirect.rgb + ambient + reflectedColor;

    //////////////////////////////////////
    // IF UNLIT MATERIAL
    //////////////////////////////////////
        } else {
            color = g_albedo;
        }

    //Fog ________________________________
        if(scene.enableFog) {
            float f = computeFog(1.0 - g_depth);
            color = f * color + (1 - f) * scene.fogColor.rgb;
        }

        outColor = vec4(color, 1.0);

    } else { //DEBUG MODE
        if(ALBEDO_OUTPUT)
            outColor = vec4(g_albedo, 1.0);
        if(NORMALS_OUTPUT)
            outColor = vec4((camera.invView * vec4(g_normal.xyz, 0.0)).xyz, 1.0);
        if(POSITION_OUTPUT)
            outColor = vec4((camera.invView * vec4(g_pos.xyz, 1.0)).xyz, 1.0);
        if(MATERIAL_OUTPUT)
            outColor = g_material;
        if(SSAO_OUTPUT)
            outColor = vec4(SSAO, SSAO, SSAO, 1.0);
    }

    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0 && LIGHTING_MODE)
        outBrightColor = vec4(color, 1.0);
    else
        outBrightColor = vec4(0.0, 0.0, 0.0, 1.0);

}


//Populate BSDF ________________________
                // DisneyBSSDF bsdf;
                // bsdf.albedo = g_albedo;
                // bsdf.opacity = g_opacity;
                // bsdf.normal = g_normal;
                // bsdf.mask = g_material.w;
                // bsdf.thickness = thickness;
            // bsdf.roughness = g_material.x;
            // bsdf.ao = g_material.z;

            // //Direct Component ________________________
            //     for(int i = 0; i < scene.numLights; i++) {
            //         //If inside liught area influence
            //         if(isInAreaOfInfluence(scene.lights[i].position, g_pos, scene.lights[i].areaEffect, int(scene.lights[i].type))) {

            //             //Direct Component ________________________
            //             vec3 lighting = vec3(0.0);
            //             float camDist = length(g_pos);
            //             // lighting = evalDisneyBSSDF(scene.lights[i].type != DIRECTIONAL_LIGHT ? normalize(scene.lights[i].position - g_pos) : normalize(-scene.lights[i].position.xyz), //wi
            //             // normalize(-g_pos),                                                                                           //wo
            //             // scene.lights[i].color * computeAttenuation(scene.lights[i].position, g_pos, scene.lights[i].areaEffect, int(scene.lights[i].type)) * scene.lights[i].intensity,              //radiance
            //             // v_uv, camDist, positionBuffer, materialBuffer, materialBuffer, bsdf);

            //             //Visibility Component ________________________
                        // lighting *= evalVisibility(i,modelPos);
            //            
            //             direct += lighting;
            //         }
            //     }