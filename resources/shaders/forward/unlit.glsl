#shader vertex
#version 460

//Input VBO
layout(location = 0) in vec3 pos;


//Output
layout(location = 0) out vec3 fragColor;
layout(location = 1) out int affectedByFog;


//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} camera;
layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 color;
    vec4 otherParams;
} object;
layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec4 color;
    vec2 tile;
    float hasColorTexture;
    float hasOpacityTexture;
} material;

void main() {
    gl_Position = camera.viewProj * object.model * vec4(pos, 1.0);
    fragColor = material.color.rgb;
    affectedByFog = int(object.otherParams.x);
}

#shader fragment
#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 1) in flat int affectedByFog;


layout(set = 0, binding = 1) uniform SceneUniforms {
    vec4 fogColor; // w is for exponent
    vec4 fogParams; //x for min, y for max, zw unused.
    vec4 ambientColor;
    vec4 lightDirection; //w for sun power
    vec4 lightColor;
} scene;
layout(set = 1, binding = 1) uniform MaterialUniforms {
     vec4 color;
    vec2 tile;
    bool alphaTest;
    float hasColorTexture;
    float hasOpacityTexture;
} material;

layout(location = 0) out vec4 outColor;

const float EPSILON = 0.1;

float computeFog() {
    float z = (2.0 * scene.fogParams.x) / (scene.fogParams.y + scene.fogParams.x - gl_FragCoord.z * (scene.fogParams.y - scene.fogParams.x));
    return exp(-scene.fogParams.z * z);
}

void main() {

    vec3 color;
    if(affectedByFog==1){
        float f = computeFog();
        color = f * fragColor.rgb + (1 - f) * scene.fogColor.rgb;
    }else{
        color = fragColor.rgb;
    }

    outColor = vec4(color, 1.0);

    //  float gamma = 2.2;
    // outColor.rgb = pow(outColor.rgb, vec3(1.0 / gamma));

    if(material.alphaTest)
        if(material.color.a<1-EPSILON)discard;
}