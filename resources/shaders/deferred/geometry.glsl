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
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec4 v_currClip;
layout(location = 4) out vec4 v_prevClip;
layout(location = 5) out mat3 v_TBN;

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec4 slot1;
    vec4 slot2;
    vec4 slot3;
    vec4 slot4;
    vec4 slot5;
    vec4 slot6;
    vec4 slot7;
    vec4 slot8;
} material;

void main() {

     // World position
    vec4 worldPos = object.model * vec4(pos, 1.0);

    // Current frame and Previus
    vec4 currClip = camera.viewProj * worldPos;
    v_currClip = currClip;
    v_prevClip = camera.prevViewProj * worldPos; 

    //Clip space
    gl_Position = currClip; 

    // currClip.xy += camera.jitter.xy * currClip.w;

    v_uv = vec2(uv.x * material.slot2.x, (1 - uv.y) * material.slot2.y); //Tiling

    mat4 mv = camera.view * object.model;
    v_pos = (mv * vec4(pos, 1.0)).xyz;

    v_normal = normalize(mat3(transpose(inverse(mv))) * normal);

    //Computing Normal Space 
    if(int(material.slot5.x) == 1) { //If has normal texture
        vec3 T = normalize(vec3(mv * vec4(tangent, 0.0)));
        vec3 N = normalize(vec3(mv * vec4(normal, 0.0)));
        vec3 B = cross(N, T);
        v_TBN = mat3(T, B, N);
    }

    //Computing velocity buffer
    v_currClip = gl_Position; 
    v_prevClip = camera.prevViewProj * object.model * vec4(pos, 1.0);
}

#shader fragment
#version 460
#include material_defines.glsl

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec4 v_currClip;
layout(location = 4) in vec4 v_prevClip;
layout(location = 5) in mat3 v_TBN;

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec4 slot1;
    vec4 slot2;
    vec4 slot3;
    vec4 slot4;
    vec4 slot5;
    vec4 slot6;
    vec4 slot7;
    vec4 slot8;
} material;

layout(set = 2, binding = 0) uniform sampler2D albedoTex;
layout(set = 2, binding = 1) uniform sampler2D normalTex;
layout(set = 2, binding = 2) uniform sampler2D materialText1;
layout(set = 2, binding = 3) uniform sampler2D materialText2;
layout(set = 2, binding = 4) uniform sampler2D materialText3;
layout(set = 2, binding = 5) uniform sampler2D materialText4;

//Output
layout(location = 0) out vec4 outNormal; //16F
layout(location = 1) out vec4 outAlbedo; //U8
layout(location = 2) out vec4 outMaterial; //U8
layout(location = 3) out vec4 outVelocityEmissive; //16F

#define EPSILON 0.1

///////////////////////////////////////////
//Surface Global properties
///////////////////////////////////////////
vec3 g_albedo = vec3(0.0);
float g_opacity = 1.0;
vec3 g_normal = vec3(0.0);
vec4 g_material = vec4(0.0);
vec3 g_emissive = vec3(0.0);
float g_fresnelThreshold = 0.0;

void setupSurfaceProperties() {

    //Depending on material
    if(material.slot8.w == PHYSICAL_MATERIAL) {

        //Setting input surface properties
        g_albedo = int(material.slot4.w) == 1 ? mix(material.slot1.rgb, texture(albedoTex, v_uv).rgb, material.slot3.x) : material.slot1.rgb;
        g_opacity = int(material.slot4.w) == 1 ? mix(material.slot1.w, texture(albedoTex, v_uv).a, material.slot6.z) : material.slot1.w;
        g_normal = int(material.slot5.x) == 1 ? normalize((v_TBN * (texture(normalTex, v_uv).rgb * 2.0 - 1.0))) : normalize(v_normal);

        if(int(material.slot6.x) == 1) {
            vec4 mask = texture(materialText1, v_uv).rgba; //Correction linearize color
            if(int(material.slot6.y) == 0) { //HDRP UNITY
	    	    //Unity HDRP uses glossiness not roughness pipeline, so it has to be inversed
                g_material.r = 1.0 - mix(material.slot3.w, mask.a, material.slot4.x);
                g_material.g = mix(material.slot3.y, mask.r, material.slot3.z);
                g_material.b = mix(material.slot4.y, mask.g, material.slot4.z);
            } else if(int(material.slot6.y) == 1) { //UNREAL
                g_material.r = mask.r;
                g_material.g = mask.b;
                g_material.b = mask.g;
            } else if(int(material.slot6.y) == 2) { //URP UNITY
                // TO DO ...
            }
        } else {
            g_material.r = material.slot5.y == 1 ? mix(material.slot3.w, texture(materialText1, v_uv).r, material.slot4.x) : material.slot3.w; //Roughness
            g_material.g = material.slot5.z == 1 ? mix(material.slot3.y, texture(materialText2, v_uv).r, material.slot3.z) : material.slot3.y; //Metalness
            g_material.b = material.slot5.w == 1 ? mix(material.slot4.y, texture(materialText3, v_uv).r, material.slot4.z) : material.slot4.y; //AO
        }

        g_emissive = material.slot6.w == 1 ? mix(material.slot7.rgb, texture(materialText4, v_uv).rgb, material.slot7.w) : material.slot7.rgb;
        g_emissive *= material.slot8.x;

        g_fresnelThreshold = material.slot8.y;

        g_material.w = PHYSICAL_MATERIAL;

    }
    if(material.slot8.w == UNLIT_MATERIAL) {
        g_albedo = int(material.slot2.w) == 1 ? texture(albedoTex, v_uv).rgb : material.slot1.rgb;
        g_material.w = UNLIT_MATERIAL;
    }
    if(material.slot8.w == HAIR_STRAND_MATERIAL) {
        g_albedo = material.slot1.rgb;
        g_material.r = material.slot2.w; //Roughness
        g_material.g = material.slot3.y; //Shift
        g_material.w = HAIR_STRAND_MATERIAL;
    }
    if(material.slot8.w == PHONG_MATERIAL) {
        // TBD .........
        g_material.w = PHONG_MATERIAL;
    }
    if(material.slot8.w == SKIN_MATERIAL) {

        //Setting skin surface properties
        g_albedo = int(material.slot4.w) == 1 ? mix(material.slot1.rgb, texture(albedoTex, v_uv).rgb, material.slot3.x) : material.slot1.rgb;
        g_normal = int(material.slot5.x) == 1 ? normalize((v_TBN * (texture(normalTex, v_uv).rgb * 2.0 - 1.0))) : normalize(v_normal);

        g_material.r = material.slot5.y == 1 ? mix(material.slot3.w, texture(materialText1, v_uv).r, material.slot4.x) : material.slot3.w; //Roughness
        g_material.g = material.slot6.x == 1 ? texture(materialText3, v_uv).r : 0.0;
        g_material.b = material.slot5.w == 1 ? mix(material.slot4.y, texture(materialText2, v_uv).r, material.slot4.z) : material.slot4.y; //AO

        g_fresnelThreshold = material.slot8.y;

        g_material.w = SKIN_MATERIAL;
    }

}

void main() {

    setupSurfaceProperties();

    if(int(material.slot2.z) == 1) {
        if(g_opacity < 1 - EPSILON)
            discard;
    } //Alpha test

    // ENCODING Emissive Color on Albedo
    float emissiveStrength = max(g_emissive.r, max(g_emissive.g, g_emissive.b));
    vec3 mixedColor = g_albedo + (g_emissive / max(emissiveStrength, 0.0001));
    mixedColor = clamp(mixedColor, 0.0, 1.0);

    //Compute velocity
    vec3 currNDC = v_currClip.xyz / v_currClip.w;
    currNDC.xy = (currNDC.xy+1)/2.0f;

    vec3 prevNDC = v_prevClip.xyz / v_prevClip.w;
    prevNDC.xy = (prevNDC.xy+1)/2.0f;

    vec2 velocity = currNDC.xy - prevNDC.xy;

    // ----- OUTS --------
    outAlbedo = vec4(mixedColor, g_opacity);
    outNormal = vec4(g_normal, v_currClip.z);
    outMaterial = g_material; //w material ID
    outVelocityEmissive = vec4(velocity, emissiveStrength, 0.0); //w Fresnel Threshold 

}
