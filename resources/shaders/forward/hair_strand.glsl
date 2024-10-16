#shader vertex
#version 460 core
#include object.glsl

//Input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 color;

//Output
layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_tangent;

void main() {

    gl_Position = object.model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(object.model))) * tangent);
    v_color = color;

}

#shader geometry
#version 460 core
#include camera.glsl

//Setup
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

//Input
layout(location = 0) in vec3 v_color[];
layout(location = 1) in vec3 v_tangent[];

//Uniforms
layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;
} material;

//Output
layout(location = 0) out vec3 g_pos;
layout(location = 1) out vec3 g_modelPos;
layout(location = 2) out vec3 g_normal;
layout(location = 3) out vec3 g_modelNormal;
layout(location = 4) out vec2 g_uv;
layout(location = 5) out vec3 g_dir;
layout(location = 6) out vec3 g_modelDir;
layout(location = 7) out vec3 g_color;
layout(location = 8) out vec3 g_origin;

float thickness = 0.02;

void emitQuadPoint(
    vec4 origin,
    vec4 right,
    float offset,
    vec3 forward,
    vec3 normal,
    vec2 uv,
    int id
) {

    vec4 newPos = origin + right * offset; //Model space
    gl_Position = camera.viewProj * newPos;
    g_dir = normalize(mat3(transpose(inverse(camera.view))) * v_tangent[id]);
    g_modelDir = v_tangent[id];
    g_color = v_color[id];
    g_pos = (camera.view * newPos).xyz;
    g_modelPos = newPos.xyz;
    g_uv = uv;
    g_normal = normalize(mat3(transpose(inverse(camera.view))) * normal);
    g_modelNormal = normal;
    g_origin = (camera.view * origin).xyz;

    EmitVertex();
}

void main() {

        //Model space --->>>

    vec4 startPoint = gl_in[0].gl_Position;
    vec4 endPoint = gl_in[1].gl_Position;

    vec4 view0 = vec4(camera.position.xyz, 1.0) - startPoint;
    vec4 view1 = vec4(camera.position.xyz, 1.0) - endPoint;

    vec3 dir0 = v_tangent[0];
    vec3 dir1 = v_tangent[1];

    vec4 right0 = normalize(vec4(cross(dir0.xyz, view0.xyz), 0.0));
    vec4 right1 = normalize(vec4(cross(dir1.xyz, view1.xyz), 0.0));

    vec3 normal0 = normalize(cross(right0.xyz, dir0.xyz));
    vec3 normal1 = normalize(cross(right1.xyz, dir1.xyz));

        //<<<----

    float halfLength = material.thickness * 0.5;

    emitQuadPoint(startPoint, right0, halfLength, dir0, normal0, vec2(1.0, 0.0), 0);
    emitQuadPoint(endPoint, right1, halfLength, dir1, normal1, vec2(1.0, 1.0), 1);
    emitQuadPoint(startPoint, -right0, halfLength, dir0, normal0, vec2(0.0, 0.0), 0);
    emitQuadPoint(endPoint, -right1, halfLength, dir1, normal1, vec2(0.0, 1.0), 1);

}

#shader fragment
#version 460 core
#include light.glsl
#include scene.glsl
#include camera.glsl
#include marschner_fit.glsl
#include shadow_mapping.glsl


//Input
layout(location = 0) in vec3 g_pos;
layout(location = 1) in vec3 g_modelPos;
layout(location = 2) in vec3 g_normal;
layout(location = 3) in vec3 g_modelNormal;
layout(location = 4) in vec2 g_uv;
layout(location = 5) in vec3 g_dir;
layout(location = 6) in vec3 g_modelDir;
layout(location = 7) in vec3 g_color;
layout(location = 8) in vec3 g_origin;

//Uniforms
layout(set = 0, binding = 2) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 3) uniform sampler2D ssaoMap;


layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 otherParams;
} object;

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;

    float Rpower;
    float TTpower;
    float TRTpower;
    float roughness;

    float scatter;
    float shift;
    float ior;
    bool glints;

    bool useScatter;
    bool coloredScatter;
    bool r;
    bool tt;

    bool trt;
    bool occlusion;
} material;

// uniform sampler2D u_noiseMap;
// uniform sampler2D u_depthMap;
// uniform samplerCube u_irradianceMap;
// uniform bool u_useSkybox;
// uniform vec3 u_BVCenter;

float scatterWeight = 0.0;



layout(location = 0) out vec4 fragColor;


//Real-time Marschnerr
vec3 computeLighting(float beta, float shift, LightUniform light, bool r, bool tt, bool trt) {

  //--->>>View space
    vec3 wi = normalize(light.position.xyz - g_pos);   //Light vector
    vec3 n = g_normal;                                 //Strand shading normal
    vec3 v = normalize(-g_pos);                         //Camera vector
    vec3 u = normalize(g_dir);                          //Strand tangent/direction

  //Betas
    float betaR = beta * beta;
    float betaTT = 0.5 * betaR;
    float betaTRT = 2.0 * betaR;


  //Theta & Phi
    float sinThetaWi = dot(wi, u);
    float sinThetaV = dot(v, u);
    vec3 wiPerp = wi - sinThetaWi * u;
    vec3 vPerp = v - sinThetaV * u;
    float cosPhiD = dot(wiPerp, vPerp) / sqrt(dot(wiPerp, wiPerp) * dot(vPerp, vPerp));

  // Diff
    float thetaD = (asin(sinThetaWi) - asin(sinThetaV)) * 0.5;
    float cosThetaD = cos(thetaD);
    float sinThetaD = sin(thetaD);

    float R = r ? M(sinThetaWi + sinThetaV - shift * 2.0, betaR) * NR(wi, v, cosPhiD, material.ior) / 0.25 : 0.0;
    vec3 TT = tt ? M(sinThetaWi + sinThetaV + shift, betaTT) * NTT(sinThetaD, cosThetaD, cosPhiD, material.ior, material.baseColor) : vec3(0.0);
    vec3 TRT = trt ? M(sinThetaWi + sinThetaV + shift * 4.0, betaTRT) * NTRT(sinThetaD, cosThetaD, cosPhiD,material.ior, material.baseColor) : vec3(0.0);

    vec3 albedo = material.baseColor;
    vec3 specular = (R * material.Rpower + TT * material.TTpower + TRT * material.TRTpower) / max(0.2, cosThetaD * cosThetaD);
    vec3 radiance = light.color * light.intensity;

    return (specular + albedo) * radiance;

}



float computeShadow(LightUniform light, int lightId) {

    vec4 posLightSpace = light.viewProj * vec4(g_modelPos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w; //For x,y and Z

    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    vec3 lightDir = normalize(light.position.xyz - g_pos);
    float bias = max(light.shadowBias * 5.0 * (1.0 - dot(g_dir, lightDir)), light.shadowBias);  //Modulate by angle of incidence

    return filterPCF(shadowMap,lightId,int(light.pcfKernel),  light.kernelRadius,projCoords, bias);

}



// vec3 multipleScattering(vec3 n){

//    vec3 l = normalize(u_scene.lightPos.xyz- g_pos);  //Light vector

//   float wrapLight = (dot(n,l)+1.0)/(4.0*PI);
//   vec3 scattering = sqrt(material.baseColor) * wrapLight * pow(material.baseColor/getLuminance(u_scene.lightColor),vec3(scatterWeight));
//   return scattering;

// }



vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 computeAmbient(vec3 n) {

    vec3 ambient;
    // if(u_useSkybox){

    //     vec3 irradiance = texture(u_irradianceMap, n).rgb*u_scene.ambientIntensity;

    //     ambient = computeLighting(material.roughness+0.2,material.shift,irradiance, 
    //     material.r,
    //     false,
    //     material.trt);

    // }else{

    ambient = (scene.ambientIntensity * scene.ambientColor) * material.baseColor;

    // }
  // ambient = fn;
    return ambient;
}

vec3 toneMapReinhard(vec3 color, float exposure) {
    vec3 mapped = exposure * color;
    return mapped / (1.0 + mapped);
}

void main() {

    //DIRECT LIGHTING ..............

    vec3 color = vec3(0.0);
    for(int i = 0; i < scene.numLights; i++) {
        //If inside liught area influence
        if(isInAreaOfInfluence(scene.lights[i], g_pos)) {

            vec3 lighting = computeLighting(material.roughness, material.shift, scene.lights[i], material.r, material.tt, material.trt);

            if(int(object.otherParams.y) == 1 && scene.lights[i].data.w == 1) {
                lighting *= (1.0 - computeShadow(scene.lights[i], i));
                //     // if(material.useScatter && material.coloredScatter)
                //     //     color+= multipleScattering(n2);
            }

            color += lighting;
        }
    }


    vec3 fakeNormal = vec3(0.0);

    //AMBIENT COMPONENT ...............

    vec3 ambient = computeAmbient(fakeNormal);
    color += ambient;

    if(int(object.otherParams.x) == 1 && scene.enableFog) {
        float f = computeFog();
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    fragColor = vec4(color, 1.0);

}