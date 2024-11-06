#shader vertex
#version 450
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

    vec3 albedo;
    float opacity;
    vec2 tileUV;
    bool alphaTest;
    float unused;

    float albedoWeight;
    float metalness;
    float metalnessWeight;
    float roughness;
    float roughnessWeight;
    float occlusion;
    float occlusionWeight;

    bool hasAlbdoTexture;
    bool hasNormalTexture;
    bool hasRoughnessTexture;
    bool hasMetallicTexture;
    bool hasAOTexture;
    bool hasMaskTexture;

    int maskType;

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
#version 450
#include camera.glsl
#include light.glsl
#include scene.glsl
#include object.glsl
#include utils.glsl
#include shadow_mapping.glsl
#include fresnel.glsl
#include ssao.glsl
#include IBL.glsl
#include reindhart.glsl
#include BRDFs/schlick_smith_BRDF.glsl

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


layout(set = 0, binding = 2) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 4) uniform samplerCube irradianceMap;


layout(set = 1, binding = 1) uniform MaterialUniforms {
     vec3 albedo;
    float opacity;
    vec2 tileUV;
    bool alphaTest;
    bool blending;
    float albedoWeight;
    float metalness;
    float metalnessWeight;
    float roughness;
    float roughnessWeight;
    float occlusion;
    float occlusionWeight;
    bool hasAlbdoTexture;
    bool hasNormalTexture;
    bool hasRoughnessTexture;
    bool hasMetallicTexture;
    bool hasAOTexture;
    bool hasMaskTexture;
    int maskType;
    float opacityWeight;
} material;


layout(set = 2, binding = 0) uniform sampler2D albedoTex;
layout(set = 2, binding = 1) uniform sampler2D normalTex;
layout(set = 2, binding = 2) uniform sampler2D maskRoughTex;
layout(set = 2, binding = 3) uniform sampler2D metalTex;
layout(set = 2, binding = 4) uniform sampler2D occlusionTex;


//BRDF Definiiton
SchlickSmithBRDF brdf;


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
        if(isInAreaOfInfluence(scene.lights[i], v_pos)){

            vec3 lighting =evalSchlickSmithBRDF( 
                scene.lights[i].type != 1 ? normalize(scene.lights[i].position - v_pos) : normalize(scene.lights[i].data.xyz), //wi
                normalize(-v_pos),                                                                                           //wo
                scene.lights[i].color * computeAttenuation( scene.lights[i], v_pos) *  scene.lights[i].intensity,              //radiance
                brdf
                );


            if(int(object.otherParams.y) == 1 && scene.lights[i].data.w == 1) {
                lighting *= (1.0 - computeVarianceShadow(shadowMap,scene.lights[i],i,v_modelPos));

            }

        color += lighting;
        }
    }

    //Ambient component ___________________________________________________________________
    vec3 ambient;
    if(scene.useIBL){
        ambient = computeAmbient(
            irradianceMap,
            scene.envRotation,
            v_modelNormal,
            normalize(camera.position.xyz-v_modelPos),
            brdf.albedo,
            brdf.F0,
            brdf.metalness,
            brdf.roughness,
            scene.ambientIntensity);
    }else{
        ambient = (scene.ambientIntensity * scene.ambientColor) * brdf.albedo;
    }
    //Ambient occlusion ___________________________________________________________________
    color+=ambient * brdf.ao;

    //Fog ___________________________________________________________________
    if(int(object.otherParams.x) == 1 && scene.enableFog) {
        float f = computeFog(gl_FragCoord.z);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    //Blending
    outColor = vec4(color, material.blending ? brdf.opacity: 1.0);


    //Postprocess
	color = reindhartTonemap(color);

}