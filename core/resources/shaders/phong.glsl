#shader vertex
#version 450

//Input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

//Output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec3 v_lightPos;
layout(location = 4) out int v_affectedByFog;
layout(location = 5) out int v_receiveShadows;
layout(location = 6) out vec3 v_modelPos;

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
    vec3 color;
    float opacity;
    float shininess;
    float glossiness;
    vec2 tileUV;
} material;

void main() {
    gl_Position = camera.viewProj * object.model * vec4(pos, 1.0);

    //OUTS
    mat4 mv = camera.view * object.model;
    v_pos = (mv * vec4(pos, 1.0)).xyz;
    v_normal = normalize(mat3(transpose(inverse(mv))) * normal);
    v_lightPos = scene.lightType == 0 ? (camera.view * vec4(scene.lighPosition, 1.0)).xyz : (camera.view * vec4(scene.lightData.xyz, 0.0)).xyz;
    v_affectedByFog = int(object.otherParams.x);
    v_uv = vec2(uv.x * material.tileUV.x, uv.y * material.tileUV.y);
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
    vec3 color;
    float opacity;
    float shininess;
    float glossiness;
    vec2 tileUV;

    bool hasColorTexture;
    bool hasOpacityTexture;
    bool hasNormalTexture;
    bool hasGlossinessTexture;

} material;
layout(set = 2, binding = 0) uniform sampler2D shadowMap;
// layout(set = 2, binding = 1) uniform sampler2D normalTex;

float computeFog() {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - gl_FragCoord.z * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * z);
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

    int edge = kernelSize/2;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float currentDepth = coords.z;

    float shadow = 0.0;

    for(int x = -edge; x <= edge; ++x) {
        for(int y = -edge; y <= edge; ++y) {
            float pcfDepth = texture(shadowMap, coords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow /= (kernelSize*kernelSize);

}

float computeShadow() {

    vec4 pos_lightSpace = scene.lightViewProj * vec4(v_modelPos.xyz, 1.0);

    vec3 projCoords = pos_lightSpace.xyz / pos_lightSpace.w;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    float bias = 0.1;
    return filterPCF(7,projCoords,bias);

}

vec3 phong() {

    vec3 lightDir = scene.lightType == 0 ? normalize(v_lightPos - v_pos) : normalize(v_lightPos);
    vec3 viewDir = normalize(-v_pos);
    vec3 halfVector = normalize(lightDir + viewDir);

    vec3 diffuse = clamp(dot(lightDir, v_normal), 0.0, 1.0) * scene.lightColor.rgb;

    //Blinn specular term
    vec3 specular = pow(max(dot(v_normal, halfVector), 0.0), material.glossiness) * material.shininess * scene.lightColor.rgb;

    vec3 color = material.hasColorTexture ? texture(shadowMap, v_uv).rgb : material.color.rgb;

    float att = computeAttenuation();

    return (diffuse + specular) * color * att * scene.lightIntensity;

}

void main() {

    vec3 color = phong();
    if(v_receiveShadows == 1)
        color *= (1.0 - computeShadow());

    vec3 ambient = scene.ambientIntensity * scene.ambientColor * 0.005;
    color += ambient;

    if(v_affectedByFog == 1) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    outColor = vec4(color, 1.0);

    float gamma = 2.2;
    outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));

}