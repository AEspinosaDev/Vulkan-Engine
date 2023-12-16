#shader vertex
#version 450

//Input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

//Output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec3 v_color;
layout(location = 3) out vec3 v_lightPos;
layout(location = 4) out int v_affectedByFog;

//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
} camera;
layout(set = 0, binding = 1) uniform SceneUniforms {
    vec4 fogColor; // w is for exponent
    vec4 fogParams; //x for min, y for max, zw unused.
    vec4 ambientColor;
    vec4 lightDirection; //w for sun power
    vec4 lightColor;
} scene;
layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 color;
    vec4 otherParams;
} object;

void main() {
    gl_Position = camera.viewProj * object.model * vec4(pos, 1.0);

    //OUTS
    mat4 mv = camera.view * object.model;
    v_pos = (mv * vec4(pos, 1.0)).xyz;
    v_normal = normalize(mat3(transpose(inverse(mv))) * normal);
    v_color = object.color.rgb;
    v_lightPos = (camera.view * vec4(scene.lightDirection.xyz, 1.0)).xyz;
    v_affectedByFog = int(object.otherParams.x);
}

#shader fragment
#version 450

//Input
layout(location = 0) in vec3 v_pos;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec3 v_color;
layout(location = 3) in vec3 v_lightPos;
layout(location = 4) in flat int v_affectedByFog;

//Output
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform SceneUniforms {
    vec4 fogColor; // w is for exponent
    vec4 fogParams; //x for min, y for max, zw unused.
    vec4 ambientColor;
    vec4 lightDirection; //w for sun power
    vec4 lightColor;
} scene;

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec4 color;
    vec4 params; //x shininess //y glossiness 
} object;

float computeFog() {
    float z = (2.0 * scene.fogParams.x) / (scene.fogParams.y + scene.fogParams.x - gl_FragCoord.z * (scene.fogParams.y - scene.fogParams.x));
    return exp(-scene.fogParams.z * z);
}

float computeAttenuation(){

    return 0.0;
}

vec3 phong() {

    vec3 lightDir = normalize(v_lightPos - v_pos);
    vec3 viewDir = normalize(-v_pos);
    vec3 halfVector = normalize(lightDir+viewDir);

    vec3 ambient = scene.ambientColor.w*scene.ambientColor.rgb;
    vec3 diffuse = clamp(dot(lightDir, v_normal), 0.0, 1.0)*scene.lightColor.rgb;
    vec3 specular = pow(max(dot(v_normal,halfVector),0.0),40.0f)*10.0f*scene.lightColor.rgb;

    return (ambient+diffuse+specular)*v_color.rgb;

}

void main() {

    vec3 color = phong();
    if(v_affectedByFog == 1) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    } 

    outColor = vec4(color, 1.0);
}