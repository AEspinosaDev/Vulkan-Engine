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
layout(location = 5) out vec3 v_modelPos;

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
    v_lightPos = (camera.view * vec4(scene.lighPosition, 1.0)).xyz;
    v_affectedByFog = int(object.otherParams.x);
    v_uv = vec2(uv.x * material.tileUV.x, uv.y * material.tileUV.y);

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
layout(location = 5) in vec3 v_modelPos;

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
layout(set = 2, binding = 0) uniform sampler2D colorTex;
// layout(set = 2, binding = 1) uniform sampler2D normalTex;

float computeFog() {
    float z = (2.0 * scene.fogMinDistance) / (scene.fogMaxDistance + scene.fogMinDistance - gl_FragCoord.z * (scene.fogMaxDistance - scene.fogMinDistance));
    return exp(-scene.fogIntensity * z);
}

float computeAttenuation() {

    return 0.0;
}

float computeShadow() {

   vec4 pos_lightSpace = scene.lightViewProj * vec4(v_modelPos.xyz, 1.0);

    // perform perspective divide
    vec3 projCoords = pos_lightSpace.xyz / pos_lightSpace.w;
    // Commenting out unnecessary transformation
    // projCoords = projCoords * 0.5 + 0.5;

    // Clamping values to [0, 1] range
    float closestDepth = texture(colorTex, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Uncomment and modify if necessary to handle fragments outside shadow map
    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    // Uncomment and modify the bias if necessary
    // float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.00005);
    float bias = 0.1;  // You can tweak this value

    // Compare depth values with bias
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
	//return lightDir.x;

    // float shadow = 1.0;

    // if(shadowCoord.z > -1.0 && shadowCoord.z < 1.0) {
    //     float dist = texture(colorTex,).r;

    //     if(shadowCoord.w > 0.0 && dist < shadowCoord.z) {
    //         shadow = ambient;
    //     }
    // }
    // return shadow;

}

vec3 phong() {

    vec3 lightDir = normalize(v_lightPos - v_pos);
    vec3 viewDir = normalize(-v_pos);
    vec3 halfVector = normalize(lightDir + viewDir);

    vec3 ambient = scene.ambientIntensity * scene.ambientColor;
    vec3 diffuse = clamp(dot(lightDir, v_normal), 0.0, 1.0) * scene.lightColor.rgb;

    //Blinn specular term
    vec3 specular = pow(max(dot(v_normal, halfVector), 0.0), material.glossiness) * material.shininess * scene.lightColor.rgb;

    //vec3 color = material.hasColorTexture ? texture(colorTex, v_uv).rgb : material.color.rgb;
    vec3 color = material.hasColorTexture ? texture(colorTex, v_uv).rgb : material.color.rgb;

    //if(color.r>1.0f) color = vec3(1.0,1.0,1.0);

    return (ambient + diffuse + specular) * color;

}

void main() {

    vec3 color = phong();
    color *= (1.0-computeShadow());

    if(v_affectedByFog == 1) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    } 
    //outColor = texture(colorTex,v_uv).xyz;
    // outColor = vec4(texture(colorTex,v_uv).xyz, 1.0);
    outColor = vec4(color, 1.0);
}