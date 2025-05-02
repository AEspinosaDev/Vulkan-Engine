#shader vertex
#version 460 core
#include object.glsl


//Input
layout(location = 0) in vec3 position;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

//Output
layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec3 v_tangent;

void main() {

    gl_Position = object.model * vec4(position, 1.0);

    v_uv = uv;
    v_tangent = normalize(mat3(transpose(inverse(object.model))) * tangent);

  
}


#shader geometry
#version 460 core
#include camera.glsl

//Setup
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

//Input
layout(location = 0) in vec2 v_uv[];
layout(location = 1) in vec3 v_tangent[];

//Uniforms
layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;
} material;

//Output
layout(location = 0) out vec3 g_pos;
layout(location = 1) out vec3 g_tangent;
layout(location = 2) out vec2 g_uv;
layout(location = 3) out vec4 g_currClip;
layout(location = 4) out vec4 g_prevClip;


void emitQuadPoint(
    vec4 origin,
    vec4 right,
    float offset,
    vec2 uv,
    int id
) {

    vec4 newPos = origin + right * offset; //Model space
    gl_Position = camera.viewProj * newPos;
    g_tangent = normalize(mat3(transpose(inverse(camera.view))) * v_tangent[id]);
    g_pos = (camera.view * newPos).xyz;
    g_uv = uv;

    //Jitter
    vec4 currClip = camera.viewProj * origin;
    g_currClip = currClip;
    g_prevClip = camera.prevViewProj * origin; 

    EmitVertex();
}

void main() {

        //Model space --->>>

    vec4 startPoint = gl_in[0].gl_Position;
    vec4 endPoint = gl_in[1].gl_Position;

    vec4 view0 = vec4(camera.position.xyz, 1.0) - startPoint;
    vec4 view1 = vec4(camera.position.xyz, 1.0) - endPoint;

    vec3 dir0 = v_tangent[0];
    vec3 dir1 = v_tangent[1];

    vec4 right0 = normalize(vec4(cross(dir0.xyz, view0.xyz), 0.0));
    vec4 right1 = normalize(vec4(cross(dir1.xyz, view1.xyz), 0.0));

        //<<<----

    float halfLength = material.thickness * 0.5;

    emitQuadPoint(startPoint, right0, halfLength, vec2(1.0, 0.0), 0);
    emitQuadPoint(endPoint, right1, halfLength, vec2(1.0, 1.0), 1);
    emitQuadPoint(startPoint, -right0, halfLength, vec2(0.0, 0.0), 0);
    emitQuadPoint(endPoint, -right1, halfLength, vec2(0.0, 1.0), 1);

}
#shader fragment
#version 460
#include object.glsl
#include material_defines.glsl
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 g_pos;
layout(location = 1) in vec3 g_tangent;
layout(location = 2) in vec2 g_uv;
layout(location = 3) in vec4 g_currClip;
layout(location = 4) in vec4 g_prevClip;

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

layout(set = 2, binding = 0) uniform sampler2D textures[];

//Settings current object textures
int objectID = int(object.materialID);
#define ALBEDO_TEX textures[objectID * 6]
#define NORMAL_TEX textures[objectID * 6 + 1]
#define MATERIAL_TEX textures[objectID * 6 + 2]
#define MATERIAL_TEX2 textures[objectID * 6 + 3]
#define MATERIAL_TEX3 textures[objectID * 6 + 4]
#define MATERIAL_TEX4 textures[objectID * 6 + 5]

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
// vec3 g_tangent = vec3(0.0);
vec4 g_material = vec4(0.0);
vec3 g_emissive = vec3(0.0);
float g_fresnelThreshold = 0.0;

void setupSurfaceProperties() {

    //Depending on material
    if(material.slot8.w == PHYSICAL_MATERIAL) {

        //Setting input surface properties
        g_albedo = int(material.slot4.w) == 1 ? mix(material.slot1.rgb, texture(ALBEDO_TEX, g_uv).rgb, material.slot3.x) : material.slot1.rgb;
        g_opacity = int(material.slot4.w) == 1 ? mix(material.slot1.w, texture(ALBEDO_TEX, g_uv).a, material.slot6.z) : material.slot1.w;
        // g_tangent = int(material.slot5.x) == 1 ? normalize((v_TBN * (texture(NORMAL_TEX, g_uv).rgb * 2.0 - 1.0))) : normalize(g_tangent);

        if(int(material.slot6.x) == 1) {
            vec4 mask = texture(MATERIAL_TEX, g_uv).rgba; //Correction linearize color
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
            g_material.r = material.slot5.y == 1 ? mix(material.slot3.w, texture(MATERIAL_TEX, g_uv).r, material.slot4.x) : material.slot3.w; //Roughness
            g_material.g = material.slot5.z == 1 ? mix(material.slot3.y, texture(MATERIAL_TEX2, g_uv).r, material.slot3.z) : material.slot3.y; //Metalness
            g_material.b = material.slot5.w == 1 ? mix(material.slot4.y, texture(MATERIAL_TEX3, g_uv).r, material.slot4.z) : material.slot4.y; //AO
        }

        g_emissive = material.slot6.w == 1 ? mix(material.slot7.rgb, texture(MATERIAL_TEX4, g_uv).rgb, material.slot7.w) : material.slot7.rgb;
        g_emissive *= material.slot8.x;

        g_fresnelThreshold = material.slot8.y;

        g_material.w = PHYSICAL_MATERIAL;

    }
    if(material.slot8.w == UNLIT_MATERIAL) {
        g_albedo = int(material.slot2.w) == 1 ? texture(ALBEDO_TEX, g_uv).rgb : material.slot1.rgb;
        g_material.w = UNLIT_MATERIAL;
    }
    if(material.slot8.w == HAIR_STRAND_MATERIAL) {
        // g_albedo = material.slot1.rgb;
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
        g_albedo = int(material.slot4.w) == 1 ? mix(material.slot1.rgb, texture(ALBEDO_TEX, g_uv).rgb, material.slot3.x) : material.slot1.rgb;
        // g_tangent = int(material.slot5.x) == 1 ? normalize((v_TBN * (texture(NORMAL_TEX, g_uv).rgb * 2.0 - 1.0))) : normalize(g_tangent);

        g_material.r = material.slot5.y == 1 ? mix(material.slot3.w, texture(MATERIAL_TEX, g_uv).r, material.slot4.x) : material.slot3.w; //Roughness
        g_material.g = material.slot6.x == 1 ? texture(MATERIAL_TEX3, g_uv).r : 0.0;
        g_material.b = material.slot5.w == 1 ? mix(material.slot4.y, texture(MATERIAL_TEX2, g_uv).r, material.slot4.z) : material.slot4.y; //AO

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
    vec3 currNDC = g_currClip.xyz / g_currClip.w;
    currNDC.xy = (currNDC.xy+1)/2.0f;

    vec3 prevNDC = g_prevClip.xyz / g_prevClip.w;
    prevNDC.xy = (prevNDC.xy+1)/2.0f;

    vec2 velocity = currNDC.xy - prevNDC.xy;

    // ----- OUTS --------
    outAlbedo = vec4(mixedColor, g_opacity);
    outNormal = vec4(g_tangent, 1.0);
    outMaterial = g_material; //w material ID
    outVelocityEmissive = vec4(velocity, emissiveStrength, 0.0); //w Fresnel Threshold 

}