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
#include shadows.glsl
#include fresnel.glsl
#include BRDFs/cook_torrance_BRDF.glsl
#include BRDFs/marschner_BSDF.glsl
#include warp.glsl
#include raytracing.glsl
#include hashing.glsl
#include SSR.glsl
#include VXGI.glsl


//INPUT
layout(location = 0) in  vec2 v_uv;

//OUTPUT
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBrightColor;

//UNIFORMS
layout(set = 0, binding =   2) uniform sampler2DArray              shadowMap;
layout(set = 0, binding =   4) uniform samplerCube                 irradianceMap;
layout(set = 0,  binding =  5) uniform accelerationStructureEXT    TLAS;
layout(set = 0,  binding =  6) uniform sampler2D                   samplerMap;
layout(set = 0,  binding =  7) uniform sampler2D                   brdfMap;
layout(set = 0,  binding =  8) uniform sampler3D                   voxelMap;
//G-BUFFER
layout(set = 1, binding = 0) uniform sampler2D positionBuffer;
layout(set = 1, binding = 1) uniform sampler2D normalBuffer;
layout(set = 1, binding = 2) uniform sampler2D colorBuffer;
layout(set = 1, binding = 3) uniform sampler2D materialBuffer;
layout(set = 1, binding = 4) uniform sampler2D emissionBuffer;
layout(set = 1, binding = 5) uniform sampler2D preCompositionBuffer;
//TEMPORAL
layout(set = 1, binding = 6) uniform sampler2D prevBuffer;

//SETTINGS
layout(push_constant) uniform Settings {
    uint    bufferOutput;
    uint    enableAO;
    uint    AOtype;
    VXGI    vxgi;   //Voxel Based GI
    SSR     ssr;    //Screen Space Reflections
} settings;

// DEBUGGING QUERIES
#define LIGHTING_MODE   settings.bufferOutput == 0
#define ALBEDO_OUTPUT   settings.bufferOutput == 1
#define NORMALS_OUTPUT  settings.bufferOutput == 2
#define POSITION_OUTPUT settings.bufferOutput == 3
#define MATERIAL_OUTPUT settings.bufferOutput == 4
#define SSAO_OUTPUT     settings.bufferOutput == 5

//SURFACE PROPERTIES
vec3    g_pos; 
float   g_depth; 
vec3    g_normal; 
vec3    g_albedo; 
float   g_opacity;
vec4    g_material; 
vec3    g_emission; 
int     g_isReflective;
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
    vec4 emissionFresnelThreshold = texture(emissionBuffer,v_uv);
    g_emission  = emissionFresnelThreshold.rgb;
    g_isReflective = int(emissionFresnelThreshold.w);
    g_temp      = vec4(0.0);
    ///////////////////////////////////
    // PRE-COMPUTED DATA
    ///////////////////////////////////
    vec2 preComputedData    = texture(preCompositionBuffer,v_uv).rg;
    float SSAO              = preComputedData.r;
    float rtShadow          = preComputedData.g;


    vec3 color            = vec3(0.0);
    vec3 reflectedColor   = vec3(0.0);

    if(LIGHTING_MODE){
    //////////////////////////////////////
    // IF LIT MATERIAL
    //////////////////////////////////////
    if(g_material.w != UNLIT_MATERIAL){

        vec3 direct     = vec3(0.0);
        vec3 ambient    = vec3(0.0);
        vec4 indirect   = vec4(0.0);
        vec3 indirectSpecular   = vec3(0.0);

        vec3 modelPos = (camera.invView * vec4(g_pos.xyz, 1.0)).xyz;
        vec3 modelNormal = (camera.invView  * vec4(g_normal.xyz, 0.0)).xyz;
        //////////////////////////////////////
        // PHYSICAL 
        //////////////////////////////////////
        if(g_material.w == PHYSICAL_MATERIAL){
            //Populate BRDF ________________________
            CookTorranceBRDF brdf;
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


            // Indirect Component ____________________________
            if(settings.vxgi.enabled == 1){

                // Diffuse
                indirect        = diffuseVoxelGI2(voxelMap , modelPos, modelNormal, settings.vxgi, scene.maxCoord.x-scene.minCoord.x);
                indirect.rgb    = evalDiffuseCookTorranceBRDF(
                    modelNormal,
                    normalize(camera.position.xyz-modelPos), 
                    indirect.rgb, 
                    brdf);
                indirect.rgb *= (1.0 - indirect.a); //Account for occlusion

                // Specular
                vec3 specularConeDirection = reflect(-normalize(camera.position.xyz-modelPos), modelNormal);
                float voxelWorldSize =  (scene.maxCoord.x-scene.minCoord.x) / float(settings.vxgi.resolution);
	            vec3 startPos = modelPos + modelNormal * voxelWorldSize * settings.vxgi.offset;

                const float CONE_SPREAD = mix(0.005, settings.vxgi.diffuseConeSpread, brdf.roughness);
                indirectSpecular = evalSpecularCookTorranceBRDF(
                    modelNormal, 
                    normalize(camera.position.xyz-modelPos),
                    traceCone(voxelMap, startPos, specularConeDirection, settings.vxgi.maxDistance, CONE_SPREAD, voxelWorldSize ).rgb, 
                    brdfMap,
                    brdf);
                
                indirect.rgb+=indirectSpecular.rgb;
            }
            //Direct Component ________________________
            for(int i = 0; i < scene.numLights; i++) {
                    //If inside liught area influence
                    if(isInAreaOfInfluence(scene.lights[i].position, g_pos,scene.lights[i].areaEffect,int(scene.lights[i].type))){

                        //Direct Component ________________________
                        vec3 lighting = vec3(0.0);
                        lighting = evalCookTorranceBRDF( 
                            scene.lights[i].type != DIRECTIONAL_LIGHT ? normalize(scene.lights[i].position - g_pos) : normalize(-scene.lights[i].position.xyz), //wi
                            normalize(-g_pos),                                                                                           //wo
                            scene.lights[i].color * computeAttenuation(scene.lights[i].position, g_pos,scene.lights[i].areaEffect,int(scene.lights[i].type)) *  scene.lights[i].intensity,              //radiance
                            brdf
                            );

                        //Visibility Component ________________________
                        if(scene.lights[i].shadowCast == 1) {
                            if(scene.lights[i].shadowType == 0) //Classic
                                lighting *= computeShadow(shadowMap, scene.lights[i], i, modelPos);
                            if(scene.lights[i].shadowType == 1) //VSM   
                                lighting *= computeVarianceShadow(shadowMap, scene.lights[i], i, modelPos);
                            if(scene.lights[i].shadowType == 2) //Raytraced  
                                lighting *= computeRaytracedShadow(
                                    TLAS, 
                                    samplerMap,
                                    modelPos, 
                                    scene.lights[i].type != DIRECTIONAL_LIGHT ? scene.lights[i].worldPosition.xyz - modelPos : -scene.lights[i].shadowData.xyz,
                                    int(scene.lights[i].shadowData.w), 
                                    scene.lights[i].area, 
                                    scene.lights[i].type != DIRECTIONAL_LIGHT ? length(scene.lights[i].worldPosition.xyz - modelPos) : 30.0,
                                    0);
                        }
                    direct += lighting;
                    }
            }
            //Emission Component ________________________
            direct += brdf.emission;
            //Ambient Component ________________________
            if(scene.useIBL){
                mat3 rotY           = rotationY(radians(scene.envRotation));
                vec3 rotatedNormal  = normalize(rotY * modelNormal);
                vec3 irradiance     = texture(irradianceMap, rotatedNormal).rgb*scene.ambientIntensity;
                ambient = evalDiffuseCookTorranceBRDF(
                    rotatedNormal,
                    normalize(camera.position.xyz-modelPos), 
                    irradiance, 
                    brdf);
            }else{
                ambient = (scene.ambientIntensity * scene.ambientColor) * brdf.albedo;
            }
            ambient *= settings.enableAO == 1 ? settings.AOtype != 2 ? (brdf.ao * SSAO) : 1.0 - indirect.a : brdf.ao;
          
            //SSR ________________________________
            vec3 fresnel = fresnelSchlick(max(dot(g_normal, normalize(g_pos)), 0.0), brdf.F0);
            if(settings.ssr.enabled == 1 && g_isReflective == 1){
                // Reflection vector
                vec3 refl = normalize(reflect(normalize(g_pos), g_normal));

                //False Importance Sampling of BRDF Roughness
                vec3 modelPos   = vec3(camera.invView * vec4(g_pos, 1.0));
                vec3 jitt       = mix(vec3(0.0), vec3(hash0(modelPos,vec3(.8, .8, .8),19.19)), brdf.roughness);
                
                //Raymarch through depth buffer
                vec3 hitPos             = g_pos; vec2 hitCoord = vec2(0.0);
                bool hit                = raymarchVCS(settings.ssr, positionBuffer, g_pos, refl, hitCoord, hitPos);
                hitCoord                = hit ? hitCoord / textureSize(positionBuffer, 0) : vec2(-1.0f);
                // hitCoord = clamp(hitCoord,vec2(0,0),vec2(1.0));
                vec3 reflectionColour   = hit ? texture(prevBuffer, hitCoord).rgb : vec3(0.0f);

                //Control edges
                vec2 dCoords            = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - hitCoord.xy));
                float screenEdgefactor  = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
                vec3 reflectionPower    = pow(brdf.metalness, REFLECTION_FALLOFF_EXP) * screenEdgefactor * -refl.z * fresnel;
                // vec3 reflectionPower    = fresnel * -refl.z* screenEdgefactor;
                
                // mat3 rotY = rotationY(radians(scene.envRotation));
                // reflectionColour += !hit && scene.useIBL ?  texture(irradianceMap, normalize(rotY*(camera.invView*vec4(refl,0.0)).xyz)).rgb*scene.ambientIntensity : vec3(0.0);

                reflectedColor = clamp(reflectionPower, 0.0, 0.9) * reflectionColour;
                // direct =  reflectionColour;
                // direct = vec3(hitCoord,hit);
            }

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

               
                    
   
    color = direct  + indirect.rgb + ambient + reflectedColor;
    // color = direct + indirect.rgb + ambient + reflectedColor;
      
    //////////////////////////////////////
    // IF UNLIT MATERIAL
    //////////////////////////////////////
    }else{
        color = g_albedo;
    }
    
    //Fog ________________________________
    if(scene.enableFog){
        float f = computeFog(g_depth);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    outColor = vec4(color,1.0);


    }
    else{ //DEBUG MODE
        if(ALBEDO_OUTPUT) 
            outColor = vec4(g_albedo,1.0);
        if(NORMALS_OUTPUT)
            outColor = vec4((camera.invView  * vec4(g_normal.xyz, 0.0)).xyz,1.0);
        if(POSITION_OUTPUT)
            outColor = vec4((camera.invView * vec4(g_pos.xyz, 1.0)).xyz,1.0);
        if(MATERIAL_OUTPUT)
            outColor = g_material;
        if(SSAO_OUTPUT) 
            outColor = vec4(SSAO,SSAO,SSAO,1.0);
    }

    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0 && LIGHTING_MODE)
        outBrightColor = vec4(color, 1.0);
    else
        outBrightColor = vec4(0.0, 0.0, 0.0, 1.0);

}

