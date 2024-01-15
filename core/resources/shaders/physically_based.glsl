#shader vertex
#version 450

//Input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

//Output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec3 v_lightPos;
layout(location = 4) out int v_affectedByFog;
layout(location = 5) out int v_receiveShadows;
layout(location = 6) out vec3 v_modelPos;
layout(location = 7) out mat3 v_TBN;

//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} camera;
layout(set = 0, binding = 1) uniform SceneUniforms {
    vec3 fogColor;
    float fogExponent;
    float fogMinDistance;
    float fogMaxDistance;
    float fogIntensity;

    float unusedSlot1;

    vec3 ambientColor;
    float ambientIntensity;

    vec3 lighPosition;
    int lightType;
    vec3 lightColor;
    float lightIntensity;
    vec4 lightData;
    mat4 lightViewProj;

} scene;
layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 otherParams;
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

void main() {

    gl_Position = camera.viewProj * object.model * vec4(pos, 1.0);

    //OUTS
    v_uv = vec2(uv.x * material.tileUV.x, uv.y * material.tileUV.y);

    mat4 mv = camera.view * object.model;
    v_pos = (mv * vec4(pos, 1.0)).xyz;

    v_normal = normalize(mat3(transpose(inverse(mv))) * normal);

    if(material.hasNormalTexture) {
        vec3 T = -normalize(vec3(mv * vec4(tangent, 0.0)));
        vec3 N = normalize(vec3(mv * vec4(normal, 0.0)));
        vec3 B = cross(N, T);
        v_TBN = mat3(T, B, N);
    }

    v_lightPos = scene.lightType == 0 ? (camera.view * vec4(scene.lighPosition, 1.0)).xyz : (camera.view * vec4(scene.lightData.xyz, 0.0)).xyz;

    v_affectedByFog = int(object.otherParams.x);
    v_receiveShadows = int(object.otherParams.y);

    v_modelPos = (object.model * vec4(pos, 1.0)).xyz;
}

#shader fragment
#version 450

//Input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_lightPos;
layout(location = 4) in flat int v_affectedByFog;
layout(location = 5) in flat int v_receiveShadows;
layout(location = 6) in vec3 v_modelPos;
layout(location = 7) in mat3 v_TBN;

//Output
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform SceneUniforms {
    vec3 fogColor;
    float fogExponent;
    float fogMinDistance;
    float fogMaxDistance;
    float fogIntensity;

    float unusedSlot1;

    vec3 ambientColor;
    float ambientIntensity;

    vec3 lighPosition;
    int lightType;
    vec3 lightColor;
    float lightIntensity;
    vec4 lightData;
    mat4 lightViewProj;
} scene;

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

layout(set = 2, binding = 0) uniform sampler2D shadowMap;

layout(set = 2, binding = 1) uniform sampler2D albedoTex;
layout(set = 2, binding = 2) uniform sampler2D normalTex;
layout(set = 2, binding = 3) uniform sampler2D maskRoughTex;
layout(set = 2, binding = 4) uniform sampler2D metalTex;
layout(set = 2, binding = 5) uniform sampler2D occlusionTex;

//Surface global properties
vec3 g_normal;
vec3 g_albedo;
float g_opacity;
float g_roughness;
float g_metalness;
float g_ao;

//Constant
const float PI = 3.14159265359;

float computeFog() {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - gl_FragCoord.z * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * 0.01 * z);
}

float computeAttenuation() {
    if(scene.lightType != 0)
        return 1.0;

    float d = length(v_lightPos - v_pos);
    float influence = scene.lightData.x;
    float window = pow(max(1 - pow(d / influence, 2), 0), 2);

    return pow(10 / max(d, 0.0001), 2) * window;
}

float filterPCF(int kernelSize, vec3 coords, float bias) {

    int edge = kernelSize / 2;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float currentDepth = coords.z;

    float shadow = 0.0;

    for(int x = -edge; x <= edge; ++x) {
        for(int y = -edge; y <= edge; ++y) {
            float pcfDepth = texture(shadowMap, coords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow /= (kernelSize * kernelSize);

}

float computeShadow() {

    vec4 pos_lightSpace = scene.lightViewProj * vec4(v_modelPos.xyz, 1.0);

    vec3 projCoords = pos_lightSpace.xyz / pos_lightSpace.w;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    float bias = 0.1;
    return filterPCF(7, projCoords, bias);

}

//Fresnel
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//Normal Distribution 
//Trowbridge - Reitz GGX
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}
//Geometry
//Schlick - GGX
float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 computeLighting() {

    //Vector setup
    vec3 lightDir = scene.lightType == 0 ? normalize(v_lightPos - v_pos) : normalize(v_lightPos);
    vec3 viewDir = normalize(-v_pos);
    vec3 halfVector = normalize(lightDir + viewDir); //normalize(viewDir + lightDir);

	//Heuristic fresnel factor
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, g_albedo, g_metalness);

	//Radiance
    vec3 radiance = scene.lightColor * computeAttenuation() * scene.lightIntensity;

	// Cook-Torrance BRDF
    float NDF = distributionGGX(g_normal, halfVector, g_roughness);
    float G = geometrySmith(g_normal, viewDir, lightDir, g_roughness);
    vec3 F = fresnelSchlick(max(dot(halfVector, viewDir), 0.0), F0);

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - g_metalness;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(g_normal, viewDir), 0.0) * max(dot(g_normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

	// Add to outgoing radiance result
    float lambertian = max(dot(g_normal, lightDir), 0.0);
    return (kD * g_albedo / PI + specular) * radiance * lambertian;

}

void main() {

    //Setting input surface properties
    g_albedo = material.hasAlbdoTexture ? mix(material.albedo.rgb, texture(albedoTex, v_uv).rgb, material.albedoWeight) : material.albedo.rgb;
    g_opacity = material.opacity;
    g_normal = material.hasNormalTexture ? normalize(v_TBN * (texture(normalTex, v_uv).rgb * 2.0 - 1.0)) : v_normal;

    if(material.hasMaskTexture) {
        // vec4 mask = pow(texture(maskRoughTex, v_uv).rgba, vec4(2.2)); //Correction linearize color
        vec4 mask = texture(maskRoughTex, v_uv).rgba; //Correction linearize color
        if(material.maskType == 0) { //HDRP UNITY
		    //Unity HDRP uses glossiness not roughness pipeline, so it has to be inversed
            g_roughness = 1.0 - mask.a;
            g_metalness = mask.r;
            g_ao = mask.g;
        } else if(material.maskType == 1) { //UNREAL
            g_roughness = mask.r;
            g_metalness = mask.b;
            g_ao = mask.g;
        } else if(material.maskType == 2) { //URP UNITY
            // TO DO ...
        }
    } else {
        g_roughness = material.hasRoughnessTexture ? mix(material.roughness, texture(maskRoughTex, v_uv).r, material.roughnessWeight) : material.roughness;
        g_metalness = material.hasMetallicTexture ? mix(material.metalness, texture(metalTex, v_uv).r, material.metalnessWeight) : material.metalness;
        g_ao = material.hasAOTexture ? mix(material.occlusion, texture(occlusionTex, v_uv).r, material.occlusionWeight) : material.occlusion;
    }

    vec3 color = computeLighting();
    if(v_receiveShadows == 1)
        color *= (1.0 - computeShadow());

    //Ambient component
    vec3 ambient = (scene.ambientIntensity * 0.01 * scene.ambientColor) * g_albedo * g_ao;
    color += ambient;

	//Tone Up
    color = color / (color + vec3(1.0));

    if(v_affectedByFog == 1) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    outColor = vec4(color, 1.0);

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));

}