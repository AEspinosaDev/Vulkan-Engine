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
    float lightType;
    vec3 lightColor;
    float lightIntensity;
    vec4 lightData;

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
    v_lightPos = (camera.view * vec4(scene.lighPosition, 1.0)).xyz;
    v_affectedByFog = int(object.otherParams.x);
    v_uv = vec2(uv.x * material.tileUV.x, uv.y * material.tileUV.y);
}

#shader fragment
#version 450

//Input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_lightPos;
layout(location = 4) in flat int v_affectedByFog;

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
    float lightType;
    vec3 lightColor;
    float lightIntensity;
    vec4 lightData;
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
layout(set = 2, binding = 0) uniform sampler2D colorTex;
// layout(set = 2, binding = 1) uniform sampler2D normalTex;

float computeFog() {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - gl_FragCoord.z * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * z);
}

float computeAttenuation() {

    return 0.0;
}

vec3 phong() {

    vec3 lightDir = normalize(v_lightPos - v_pos);
    vec3 viewDir = normalize(-v_pos);
    vec3 halfVector = normalize(lightDir + viewDir);

    vec3 ambient = scene.ambientIntensity * scene.ambientColor;
    vec3 diffuse = clamp(dot(lightDir, v_normal), 0.0, 1.0) * scene.lightColor.rgb;

    //Blinn specular term
    vec3 specular = pow(max(dot(v_normal, halfVector), 0.0), material.glossiness) * material.shininess * scene.lightColor.rgb;

    vec3 color = material.hasColorTexture ? texture(colorTex, v_uv).rgb : material.color.rgb;

    return (ambient + diffuse + specular) * color;

}

void main() {

    vec3 color = phong();
    if(v_affectedByFog == 1) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    } 
    //outColor = texture(colorTex,v_uv).xyz;
    // outColor = vec4(texture(colorTex,v_uv).xyz, 1.0);
    outColor = vec4(color, 1.0);
}