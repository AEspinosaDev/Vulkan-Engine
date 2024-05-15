#shader vertex
#version 450

#define MAX_LIGHTS 50

//Input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

//Output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec3 v_modelPos;
layout(location = 4) out vec2 v_screenExtent;
layout(location = 5) out mat3 v_TBN;

//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec2 screenExtent;
} camera;

struct LightUniform{
   vec3 position;
    int type;
    vec3 color;
    float intensity;
    vec4 data;

    mat4 viewProj;

    float shadowBias;
    bool apiBiasEnabled;
    bool angleDependantBias;
    float pcfKernel;
};

layout(set = 0, binding = 1) uniform SceneUniforms {
    vec3 fogColor;

    bool enableSSAO;

    float fogMinDistance;
    float fogMaxDistance;
    float fogIntensity;

    bool enableFog;

    vec3 ambientColor;
    float ambientIntensity;
    LightUniform lights[MAX_LIGHTS];
    int numLights;
} scene;

layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 otherParams;
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

    v_screenExtent = camera.screenExtent;
}

#shader fragment
#version 450

#define MAX_LIGHTS 50

//Input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_modelPos;
layout(location = 4) in vec2 v_screenExtent;
layout(location = 5) in mat3 v_TBN;

//Output
layout(location = 0) out vec4 outColor;

struct LightUniform{
   vec3 position;
    int type;
    vec3 color;
    float intensity;
    vec4 data;

    mat4 viewProj;

    float shadowBias;
    bool apiBiasEnabled;
    bool angleDependantBias;
    float pcfKernel;
};

layout(set = 0, binding = 1) uniform SceneUniforms {
    vec3 fogColor;

    bool enableSSAO;

    float fogMinDistance;
    float fogMaxDistance;
    float fogIntensity;

    bool enableFog;

    vec3 ambientColor;
    float ambientIntensity;
    LightUniform lights[MAX_LIGHTS];
    int numLights;
} scene;

layout(set = 0, binding = 2) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 3) uniform sampler2D ssaoMap;

layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 otherParams;
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


layout(set = 2, binding = 0) uniform sampler2D albedoTex;
layout(set = 2, binding = 1) uniform sampler2D normalTex;
layout(set = 2, binding = 2) uniform sampler2D maskRoughTex;
layout(set = 2, binding = 3) uniform sampler2D metalTex;
layout(set = 2, binding = 4) uniform sampler2D occlusionTex;


//Surface global properties
vec3 g_normal;
vec3 g_albedo;
float g_opacity;
float g_roughness;
float g_metalness;
float g_ao;

//Constant
const float PI = 3.14159265359;
const float EPSILON = 0.1;

bool isInAreaOfInfluence(LightUniform light){
    if(light.type == 0){ //Point Light
        return length(light.position - v_pos) <= light.data.x;
    }
    else if(light.type == 2){ //Spot light
        //TO DO...
        return true;
    }
    return true; //Directional influence is total
}

float computeFog() {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - gl_FragCoord.z * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * 0.01 * z);
}

float computeAttenuation(LightUniform light) {
    if(light.type != 0)
        return 1.0;

    float d = length(light.position - v_pos);
    float influence = light.data.x;
    float window = pow(max(1 - pow(d / influence, 2), 0), 2);

    return pow(10 / max(d, 0.0001), 2) * window;
}

float filterPCF(int lightId ,int kernelSize, vec3 coords, float bias) {

    int edge = kernelSize / 2;
    vec3 texelSize = 1.0 / textureSize(shadowMap, 0);

    float currentDepth = coords.z;

    float shadow = 0.0;

    for(int x = -edge; x <= edge; ++x) {
        for(int y = -edge; y <= edge; ++y) {
            float pcfDepth = texture(shadowMap, vec3(coords.xy + vec2(x, y) * texelSize.xy,lightId)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow /= (kernelSize * kernelSize);

}

float computeShadow(LightUniform light, int lightId) {

    vec4 pos_lightSpace = light.viewProj * vec4(v_modelPos.xyz, 1.0);

    vec3 projCoords = pos_lightSpace.xyz / pos_lightSpace.w;

    projCoords.xy  = projCoords.xy * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    
    return filterPCF(lightId,int(light.pcfKernel), projCoords, light.shadowBias);

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

vec3 computeLighting(LightUniform light) {

    //Vector setup
    vec3 lightDir = light.type == 0 ? normalize(light.position - v_pos) : normalize(light.position);
    vec3 viewDir = normalize(-v_pos);
    vec3 halfVector = normalize(lightDir + viewDir); //normalize(viewDir + lightDir);

	//Heuristic fresnel factor
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, g_albedo, g_metalness);

	//Radiance
    vec3 radiance = light.color * computeAttenuation(light) * light.intensity;

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

void setupSurfaceProperties(){
  //Setting input surface properties
    g_albedo = material.hasAlbdoTexture ? mix(material.albedo.rgb, texture(albedoTex, v_uv).rgb, material.albedoWeight) : material.albedo.rgb;
    g_opacity =  material.hasAlbdoTexture ?  texture(albedoTex, v_uv).a :material.opacity;
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
}

void main() {

    setupSurfaceProperties();

    //Compute all lights
    vec3 color = vec3(0.0);
    for(int i = 0; i < scene.numLights; i++) {
        //If inside liught area influence
        if(isInAreaOfInfluence(scene.lights[i])){

            vec3 lighting =computeLighting(scene.lights[i]);
            if(int(object.otherParams.y) == 1 && scene.lights[i].data.w == 1) {
                lighting *= (1.0 - computeShadow(scene.lights[i],i));

            }

        color += lighting;
        }
    }

    //Ambient component
    float occ = scene.enableSSAO ? texture(ssaoMap,vec2(gl_FragCoord.x/v_screenExtent.x,gl_FragCoord.y/v_screenExtent.y)).r : 1.0;
    vec3 ambient = (scene.ambientIntensity * 0.01 * scene.ambientColor) * g_albedo * occ;


    color += ambient;

	//Tone Up
    color = color / (color + vec3(1.0));

    if(int(object.otherParams.x) == 1 && scene.enableFog) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    outColor = vec4(color, 1.0);

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));

    if(material.alphaTest)
        if(g_opacity<1-EPSILON)discard;

}