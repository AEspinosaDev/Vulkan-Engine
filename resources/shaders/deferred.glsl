#shader vertex
#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
} object;

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

layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out mat3 v_TBN;

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
}

#shader fragment
#version 460

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in mat3 v_TBN;

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



layout(set = 2, binding = 0) uniform sampler2D albedoTex;
layout(set = 2, binding = 1) uniform sampler2D normalTex;
layout(set = 2, binding = 2) uniform sampler2D maskRoughTex;
layout(set = 2, binding = 3) uniform sampler2D metalTex;
layout(set = 2, binding = 4) uniform sampler2D occlusionTex;


//Surface global properties
vec3 g_normal;
vec3 g_albedo;
float g_opacity;
vec3 g_material;

//Output
layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outMaterial;

const float EPSILON = 0.1;


void setupSurfaceProperties(){
  //Setting input surface properties
    g_albedo = material.hasAlbdoTexture ? mix(material.albedo.rgb, texture(albedoTex, v_uv).rgb, material.albedoWeight) : material.albedo.rgb;
    g_opacity =  material.hasAlbdoTexture ?  texture(albedoTex, v_uv).a :material.opacity;
    g_normal = material.hasNormalTexture ? normalize((v_TBN * (texture(normalTex, v_uv).rgb * 2.0 - 1.0))) : normalize( v_normal );

    if(material.hasMaskTexture) {
        // vec4 mask = pow(texture(maskRoughTex, v_uv).rgba, vec4(2.2)); //Correction linearize color
        vec4 mask = texture(maskRoughTex, v_uv).rgba; //Correction linearize color
        if(material.maskType == 0) { //HDRP UNITY
		    //Unity HDRP uses glossiness not roughness pipeline, so it has to be inversed
            g_material.r = 1.0 - mask.a;
            g_material.g = mask.r;
            g_material.b = mask.g;
        } else if(material.maskType == 1) { //UNREAL
            g_material.r  = mask.r;
            g_material.g  = mask.b;
            g_material.b = mask.g;
        } else if(material.maskType == 2) { //URP UNITY
            // TO DO ...
        }
    } else {
        g_material.r = material.hasRoughnessTexture ? mix(material.roughness, texture(maskRoughTex, v_uv).r, material.roughnessWeight) : material.roughness;
        g_material.g = material.hasMetallicTexture ? mix(material.metalness, texture(metalTex, v_uv).r, material.metalnessWeight) : material.metalness;
        g_material.b = material.hasAOTexture ? mix(material.occlusion, texture(occlusionTex, v_uv).r, material.occlusionWeight) : material.occlusion;
    }
}

void main() {

    setupSurfaceProperties();

    outPos = vec4(v_pos,gl_FragCoord.z);
    outNormal = vec4( g_normal , 1.0f );
    outAlbedo = vec4(g_albedo,g_opacity);
    outMaterial = vec4(g_material, 1.0); //w material id

     if(material.alphaTest)
        if(g_opacity<1-EPSILON)discard;
}
