#shader vertex
#version 460


layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec2 screenExtent;
} camera;


layout(location = 0) out vec2 v_uv;
layout(location = 1) out mat4 v_camView; //For shadows

void main() {
    gl_Position = vec4(pos, 1.0);

    v_uv = uv;
    v_camView = camera.view;
}

#shader fragment
#version 460
#define MAX_LIGHTS 50

layout(location = 0) in  vec2 v_uv;
layout(location = 1) in mat4 v_camView; //For shadows


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

layout(set = 3, binding = 0) uniform sampler2D positionBuffer;
layout(set = 3, binding = 1) uniform sampler2D normalBuffer;
layout(set = 3, binding = 2) uniform sampler2D albedoBuffer;
layout(set = 3, binding = 3) uniform sampler2D materialBuffer;
layout(set = 3, binding = 4) uniform AuxUniforms {
    vec4 outputType;
} aux;


layout(location = 0) out vec4 outColor;

//Constant
const float PI = 3.14159265359;

//Surface properties
vec3 g_pos;
float g_depth;
vec3 g_normal;
vec3 g_albedo;
vec4 g_material;
float g_opacity;
float g_ao;

bool isInAreaOfInfluence(LightUniform light){
    if(light.type == 0){ //Point Light
        return length(light.position - g_pos) <= light.data.x;
    }
    else if(light.type == 2){ //Spot light
        //TO DO...
        return true;
    }
    return true; //Directional influence is total
}

float computeFog() {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - g_depth * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * 0.01 * z);
}

float computeAttenuation(LightUniform light) {
    if(light.type != 0)
        return 1.0;

    float d = length(light.position - g_pos);
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

    vec4 pos_lightSpace = light.viewProj * inverse(v_camView) * vec4(g_pos.xyz, 1.0);

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

vec3 cookTorrance(LightUniform light) {

    //Vector setup
    vec3 lightDir = light.type == 0 ? normalize(light.position - g_pos) : normalize(light.position);
    vec3 viewDir = normalize(-g_pos);
    vec3 halfVector = normalize(lightDir + viewDir); //normalize(viewDir + lightDir);

	//Heuristic fresnel factor
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, g_albedo,  g_material.g);

	//Radiance
    vec3 radiance = light.color * computeAttenuation(light) * light.intensity;

	// Cook-Torrance BRDF
    float NDF = distributionGGX(g_normal, halfVector, g_material.r);
    float G = geometrySmith(g_normal, viewDir, lightDir,  g_material.r);
    vec3 F = fresnelSchlick(max(dot(halfVector, viewDir), 0.0), F0);

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - g_material.g;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(g_normal, viewDir), 0.0) * max(dot(g_normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

	// Add to outgoing radiance result
    float lambertian = max(dot(g_normal, lightDir), 0.0);
    return (kD * g_albedo / PI + specular) * radiance * lambertian;

}
vec3 phong(LightUniform light) {

    vec3 lightDir = light.type == 0 ? normalize(light.position - g_pos) : normalize(light.position);
    vec3 viewDir = normalize(-g_pos);
    vec3 halfVector = normalize(lightDir + viewDir); //normalize(viewDir + lightDir);

    vec3 diffuse = clamp(dot(lightDir, g_normal), 0.0, 1.0) * light.color.rgb;

    //Blinn specular term
    vec3 specular = pow(max(dot(g_normal, halfVector), 0.0), 0.5) * 20 *  light.color.rgb;


    // float att = computeAttenuation();

    return (diffuse + specular) * g_albedo;

}


void main()
{

    g_pos = texture(positionBuffer,v_uv).rgb;
    g_depth = texture(positionBuffer,v_uv).w;
    g_normal = normalize(texture(normalBuffer,v_uv).rgb * 2.0 - 1.0);
    g_albedo =  texture(albedoBuffer,v_uv).rgb;
    g_material = texture(materialBuffer,v_uv);
    // g_opacity = 1.0;
    g_ao =  texture(ssaoMap,v_uv).r;


   //Compute all lights
    vec3 color = vec3(0.0);
    for(int i = 0; i < scene.numLights; i++) {
        //If inside liught area influence
        if(isInAreaOfInfluence(scene.lights[i])){

            vec3 lighting =cookTorrance(scene.lights[i]);
            if( scene.lights[i].data.w == 1) {
                lighting *= (1.0 - computeShadow(scene.lights[i],i));

            }

        color += lighting;
        }
    }

    //Ambient component
    float occ = scene.enableSSAO ? g_ao : 1.0;
    vec3 ambient = (scene.ambientIntensity * 0.01 * scene.ambientColor) * g_albedo * occ ;


    color += ambient;

	//Tone Up
    color = color / (color + vec3(1.0));

    if( scene.enableFog) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    outColor = vec4(color, 1.0);

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));

    if(g_depth >= 1.0) discard; // Leave for background


    //DIFFERENT G BUFFER OUTPUTS FOR DEBUGGING PURPOSES
    switch(int(aux.outputType.x)){
            case 1:
                 outColor = vec4(texture(positionBuffer,v_uv).rgb,1.0);
                break;
            case 2:
                 outColor = vec4(texture(normalBuffer,v_uv).rgb,1.0);
                break;
            case 3:
                 outColor = vec4(texture(albedoBuffer,v_uv).rgb,1.0);
                break;
            case 4:
                 outColor = vec4(texture(materialBuffer,v_uv).rgb,1.0);
                break;
            case 5:
                 outColor = vec4(texture(ssaoMap,v_uv).rgb,1.0);
                break;
    }

}

