#shader vertex
#version 460
#include camera.glsl
#include object.glsl


//Input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

//Output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec3 v_modelNormal;
layout(location = 3) out vec2 v_uv;
layout(location = 4) out vec3 v_modelPos;
layout(location = 5) out vec2 v_screenExtent;
layout(location = 6) out mat3 v_TBN;

//Uniforms
layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3    albedo;
    float   opacity;
    vec2    tileUV;
    bool    alphaTest;
    bool    blending;
    float   albedoWeight;
    float   metalness;
    float   metalnessWeight;
    float   roughness;
    float   roughnessWeight;
    float   occlusion;
    float   occlusionWeight;
    bool    hasAlbdoTexture;
    bool    hasNormalTexture;
    bool    hasRoughnessTexture;
    bool    hasMetallicTexture;
    bool    hasAOTexture;
    bool    hasMaskTexture;
    int     maskType;
    float   opacityWeight;
    bool    hasEmissiveTexture;
    vec3    emissiveColor;
    float   emissiveWeight;
    float   emissionIntensity;
} material;

void main() {

    gl_Position = camera.viewProj * object.model * vec4(pos, 1.0);

    v_uv = vec2(uv.x * material.tileUV.x, (1-uv.y) * material.tileUV.y);

    mat4 mv = camera.view * object.model;
    v_pos = (mv * vec4(pos, 1.0)).xyz;

    v_normal = normalize(mat3(transpose(inverse(mv))) * normal);

    if(material.hasNormalTexture) {
        vec3 T = -normalize(vec3(mv * vec4(tangent, 0.0)));
        vec3 N = normalize(vec3(mv * vec4(normal, 0.0)));
        vec3 B = cross(N, T);
        v_TBN = mat3(T, B, N);
    }

    v_modelPos = (object.model * vec4(pos, 1.0)).xyz;
    v_modelNormal = normalize(mat3(transpose(inverse(object.model))) * normal);

    v_screenExtent = camera.screenExtent;

}

#shader fragment
#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

#include camera.glsl
#include light.glsl
#include scene.glsl
#include object.glsl
#include utils.glsl
#include shadows.glsl
#include fresnel.glsl
#include BRDFs/cook_torrance_BRDF.glsl
#include warp.glsl
#include raytracing.glsl

//Input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_modelNormal;
layout(location = 3) in vec2 v_uv;
layout(location = 4) in vec3 v_modelPos;
layout(location = 5) in vec2 v_screenExtent;
layout(location = 6) in mat3 v_TBN;

//Output
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBrightColor;


//Uniforms
layout(set = 0, binding = 2)    uniform sampler2DArray              shadowMap;
layout(set = 0, binding = 4)    uniform samplerCube                 irradianceMap;
layout(set = 0,  binding = 5)   uniform accelerationStructureEXT    TLAS;
layout(set = 0,  binding = 6)   uniform sampler2D                   blueNoiseMap;
layout(set = 1, binding = 1)    uniform MaterialUniforms {
    vec3    albedo;
    float   opacity;
    vec2    tileUV;
    bool    alphaTest;
    bool    blending;
    float   albedoWeight;
    float   metalness;
    float   metalnessWeight;
    float   roughness;
    float   roughnessWeight;
    float   occlusion;
    float   occlusionWeight;
    bool    hasAlbdoTexture;
    bool    hasNormalTexture;
    bool    hasRoughnessTexture;
    bool    hasMetallicTexture;
    bool    hasAOTexture;
    bool    hasMaskTexture;
    int     maskType;
    float   opacityWeight;
    bool    hasEmissiveTexture;
    vec3    emissiveColor;
    float   emissiveWeight;
    float   emissionIntensity;
} material;
layout(set = 2, binding = 0) uniform sampler2D albedoTex;
layout(set = 2, binding = 1) uniform sampler2D normalTex;
layout(set = 2, binding = 2) uniform sampler2D maskRoughTex;
layout(set = 2, binding = 3) uniform sampler2D metalTex;
layout(set = 2, binding = 4) uniform sampler2D occlusionTex;
layout(set = 2, binding = 5) uniform sampler2D emissiveTex;


//BRDF Definiiton
CookTorranceBRDF brdf;


void setupBRDFProperties(){
  //Setting input surface properties
    brdf.albedo = material.hasAlbdoTexture ? mix(material.albedo.rgb, texture(albedoTex, v_uv).rgb, material.albedoWeight) : material.albedo.rgb;
    brdf.opacity =  material.hasAlbdoTexture ?  mix(material.opacity, texture(albedoTex, v_uv).a, material.opacityWeight) :material.opacity;
    brdf.normal = material.hasNormalTexture ? normalize(v_TBN * (texture(normalTex, v_uv).rgb * 2.0 - 1.0)) : v_normal;

    if(material.hasMaskTexture) {
        // vec4 mask = pow(texture(maskRoughTex, v_uv).rgba, vec4(2.2)); //Correction linearize color
        vec4 mask = texture(maskRoughTex, v_uv).rgba; //Correction linearize color
        if(material.maskType == 0) { //HDRP UNITY
		    //Unity HDRP uses glossiness not roughness pipeline, so it has to be inversed
            brdf.roughness = 1.0 - mask.a;
            brdf.metalness = mask.r;
            brdf.ao = mask.g;
        } else if(material.maskType == 1) { //UNREAL
            brdf.roughness = mask.r;
            brdf.metalness = mask.b;
            brdf.ao = mask.g;
        } else if(material.maskType == 2) { //URP UNITY
            // TO DO ...
        }
    } else {
        brdf.roughness = material.hasRoughnessTexture ? mix(material.roughness, texture(maskRoughTex, v_uv).r, material.roughnessWeight) : material.roughness;
        brdf.metalness = material.hasMetallicTexture ? mix(material.metalness, texture(metalTex, v_uv).r, material.metalnessWeight) : material.metalness;
        brdf.ao = material.hasAOTexture ? mix(material.occlusion, texture(occlusionTex, v_uv).r, material.occlusionWeight) : material.occlusion;
    }
    brdf.F0 = vec3(0.04);
    brdf.F0 = mix(brdf.F0, brdf.albedo, brdf.metalness);

    brdf.emission =  material.hasEmissiveTexture ? mix(material.emissiveColor, texture(emissiveTex, v_uv).rgb, material.emissiveWeight) : material.emissiveColor;
    brdf.emission *= material.emissionIntensity;
}



void main() {

    // BRDF  ___________________________________________________________________
    setupBRDFProperties();

    if(material.alphaTest)
        if(brdf.opacity<1-EPSILON)discard;

    //Compute all lights ___________________________________________________________________
    vec3 color = vec3(0.0);
    for(int i = 0; i < scene.numLights; i++) {
        //If inside liught area influence
        if(isInAreaOfInfluence(scene.lights[i].position, v_pos,scene.lights[i].areaEffect,int(scene.lights[i].type))){

            vec3 lighting =evalCookTorranceBRDF( 
                scene.lights[i].type != DIRECTIONAL_LIGHT ? normalize(scene.lights[i].position - v_pos) : normalize(scene.lights[i].position.xyz), //wi
                normalize(-v_pos),                                                                                           //wo
                scene.lights[i].color * computeAttenuation(scene.lights[i].position, v_pos,scene.lights[i].areaEffect,int(scene.lights[i].type)) *  scene.lights[i].intensity,              //radiance
                brdf
                );


            if(int(object.otherParams.y) == 1 && scene.lights[i].shadowCast == 1) {
                if(scene.lights[i].shadowType == 0) //Classic
                    lighting *= computeShadow(shadowMap, scene.lights[i], i, v_modelPos);
                if(scene.lights[i].shadowType == 1) //VSM   
                    lighting *= computeVarianceShadow(shadowMap, scene.lights[i], i, v_modelPos);
                if(scene.lights[i].shadowType == 2) //Raytraced  
                    lighting *= computeRaytracedShadow(
                        TLAS, 
                        blueNoiseMap,
                        v_modelPos, 
                        scene.lights[i].type != DIRECTIONAL_LIGHT ? scene.lights[i].shadowData.xyz - v_modelPos : scene.lights[i].shadowData.xyz,
                        int(scene.lights[i].shadowData.w), 
                        scene.lights[i].area,
                        scene.lights[i].type != DIRECTIONAL_LIGHT ? length(scene.lights[i].shadowData.xyz - v_modelPos) : 30.0, 
                        0);
            }

        color += lighting;
        }
    }

    //Ambient component ___________________________________________________________________
    vec3 ambient;
    if(scene.useIBL){
        mat3 rotY           = rotationY(radians(scene.envRotation));
        vec3 rotatedNormal  = normalize(rotY * v_modelNormal);
        vec3 irradiance     = texture(irradianceMap, rotatedNormal).rgb*scene.ambientIntensity;
        ambient = evalDiffuseCookTorranceBRDF(
            rotatedNormal,
            normalize(camera.position.xyz-v_modelPos), 
            irradiance, 
            brdf);
    }else{
        ambient = (scene.ambientIntensity * scene.ambientColor) * brdf.albedo;
    }
    //Emission ___________________________________________________________________
    color += brdf.emission;
    //Ambient occlusion ___________________________________________________________________
    color += ambient * brdf.ao;
    //Fog ___________________________________________________________________
    if(int(object.otherParams.x) == 1 && scene.enableFog) {
        float f = computeFog(gl_FragCoord.z);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    //Blending
    outColor = vec4(color, material.blending ? brdf.opacity: 1.0);

    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        outBrightColor = vec4(color, 1.0);
    else
        outBrightColor = vec4(0.0, 0.0, 0.0, 1.0);



}