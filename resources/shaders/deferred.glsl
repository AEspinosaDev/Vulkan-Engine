#shader vertex
#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

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

    vec2 tileUV;
} material;

layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

void main() {

    v_pos = (camera.view * object.model * vec4(pos,1.0)).rgb;
    v_normal = normalize(mat3(transpose(inverse( camera.view * object.model))) * normal);
    gl_Position = camera.proj * vec4(v_pos,1.0);

    v_uv = vec2(uv.x * material.tileUV.x, (1-uv.y) * material.tileUV.y);
}

#shader fragment
#version 460

layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 albedo;
    float opacity;
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

    vec2 tileUV;
} material;

//Output
layout(location = 0) out vec4 outPos;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
// layout(location = 3) out vec4 outMaterial;

void main() {

    outPos = vec4(v_pos,1.0);
    outNormal = vec4(v_normal,1.0);
    outAlbedo = vec4(v_normal,1.0);
    // outMaterial = vec4(1.0f);

    // vec3 normal = v_normal * 0.5 + 0.5;
    // outNormal = vec4(
    //     1-normal.x,
    //     normal.y,
    //     1-normal.z,
    //     1.0);
}
