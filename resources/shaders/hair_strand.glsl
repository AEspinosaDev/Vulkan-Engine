#shader vertex
#version 460 core

//Input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 color;

//Uniforms
layout(set = 1, binding = 0) uniform ObjectUniforms {
    mat4 model;
    vec4 otherParams;
    int selected;
} object;

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

//Setup
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

//Input
layout(location = 0) in vec3 v_color[];
layout(location = 1) in vec3 v_tangent[];

//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec4 position;
     vec2 screenExtent;
} camera;

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
// layout(location = 9) out int g_id;

// uniform float u_thickness;
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
    // g_id = v_id[0];

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

#define MAX_LIGHTS 50

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
// layout(location = 9) in flat int g_id;

//Uniforms
layout(set = 0, binding = 0) uniform CameraUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec4 position;
     vec2 screenExtent;
} camera;

struct LightUniform {
    vec3 position;
    int type;
    vec3 color;
    float intensity;
    vec4 data;

    mat4 viewProj;

    float shadowBias;
    float kernelRadius;
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
    int SSAOType;
    bool emphasizeAO;
} scene;

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

//Constant
const float PI = 3.14159265359;

layout(location = 0) out vec4 fragColor;

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

vec3 shiftTangent(vec3 T, vec3 N, float shift) {
    vec3 shiftedT = T + shift * N;
    return normalize(shiftedT);
}

float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0;
    float linearZ = (2.0 * near * far) / (far + near - z * (far - near));
    return (linearZ - near) / (far - near);
}
///Fresnel
float fresnelSchlick(float ior, float cosTheta) {
    float F0 = ((1.0 - ior) * (1.0 - ior)) / ((1.0 + ior) * (1.0 + ior));
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Azimuthal REFLECTION
float NR(vec3 wi, vec3 wo, float cosPhi) {
    float cosHalfPhi = sqrt(0.5 + 0.5 * cosPhi);

  // return (0.25*cosHalfPhi)*fresnelSchlick(material.ior,sqrt(0.5*(1+dot(wi,wo)))); //Frostbite
    return (0.25 * cosHalfPhi) * fresnelSchlick(material.ior, sqrt(0.5 + 0.5 * dot(wi, wo))); //Epic
}
//Attenuattion
vec3 A(float f, int p, vec3 t) { //fresnel, Stage, Absorbtion
    return (1 - f) * (1 - f) * pow(f, p - 1) * pow(t, vec3(p));
}

// Azimuthal TRANSMITION
vec3 NTT(float sinThetaD, float cosThetaD, float cosPhi) {

  // float _ior = sqrt(material.ior*material.ior - sinThetaD* sinThetaD)/ cosThetaD; //Original
    float _ior = 1.19 / cosThetaD + 0.36 * cosThetaD; //Fit EPIC
    float a = 1 / _ior;

    float cosHalfPhi = sqrt(0.5 + 0.5 * cosPhi);
    float h = 1 + a * (0.6 - 0.8 * cosPhi) * cosHalfPhi; //Fit EPIC

    float power = sqrt(1 - h * h * a * a) / (2 * cosThetaD);

    float D = exp(-3.65 * cosPhi - 3.98); //Intensity distribution
    vec3 T = pow(material.baseColor, vec3(power)); //Absortion
    float F = fresnelSchlick(material.ior, cosThetaD * sqrt(1 - h * h)); //Fresnel

    return A(F, 1, T) * D;
}
// Azimuthal DOUBLE REFLECTION
vec3 NTRT(float sinThetaD, float cosThetaD, float cosPhi) {
    float h = sqrt(3.0) * 0.5; //Por que si

    float D = exp(17.0 * cosPhi - 16.78); //Intensity distribution
    vec3 T = pow(material.baseColor, vec3(0.8 / cosThetaD));
    float F = fresnelSchlick(material.ior, acos(cosThetaD * sqrt(1 - h * h))); //Fresnel CONSTANT ?

    return A(F, 2, T) * D;
}

//Longitudinal TERM
float M(float sinTheta, float roughness) {
  // return exp(-(sinTheta*sinTheta)/(2*roughness*roughness))/sqrt(2*PI*roughness); //Frostbite 
    return (1.0 / (roughness * sqrt(2 * PI))) * exp((-sinTheta * sinTheta) / (2.0 * roughness * roughness)); //Epic. sintheta = sinThetaWi+sinThetaV-alpha
}

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

//   if(material.glints){
//     float glint = texture(u_noiseMap, vec2(g_id/50000.0,g_uv.y)).r; //Make them move tangent
//     u = shiftTangent(u,n,(glint)*0.1);
//   }

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

    float R = r ? M(sinThetaWi + sinThetaV - shift * 2.0, betaR) * NR(wi, v, cosPhiD) / 0.25 : 0.0;
    vec3 TT = tt ? M(sinThetaWi + sinThetaV + shift, betaTT) * NTT(sinThetaD, cosThetaD, cosPhiD) : vec3(0.0);
    vec3 TRT = trt ? M(sinThetaWi + sinThetaV + shift * 4.0, betaTRT) * NTRT(sinThetaD, cosThetaD, cosPhiD) : vec3(0.0);

    vec3 albedo = material.baseColor;
    vec3 specular = (R * material.Rpower + TT * material.TTpower + TRT * material.TRTpower) / max(0.2, cosThetaD * cosThetaD);
    vec3 radiance = light.color * light.intensity;

    return (specular + albedo) * radiance;

}

float filterPCF(int lightID,int kernelSize, vec3 coords, float bias) {
// int lightId ,int kernelSize, float extentMultiplier, vec3 coords, float bias
    int edge = kernelSize / 2;
    vec3 texelSize = 1.0 / textureSize(shadowMap, 0);

    float currentDepth = coords.z;

    float shadow = 0.0;

    for(int x = -edge; x <= edge; ++x) {
        for(int y = -edge; y <= edge; ++y) {
            float pcfDepth = texture(shadowMap, vec3(coords.xy + vec2(x, y) * texelSize.xy ,lightID)).r;

            // //Scatter weight
            // float currentZ = linearizeDepth(currentDepth, u_scene.frustrumData.z, u_scene.frustrumData.w);
            // float shadowZ = linearizeDepth(pcfDepth, u_scene.frustrumData.z, u_scene.frustrumData.w);
            // float weight = 1.0 - clamp(exp(-material.scatter * abs(currentZ - shadowZ)), 0.0, 1.0);
            // scatterWeight += weight;

            // shadow += currentDepth - bias > pcfDepth ? 1.0 * (material.useScatter ? weight : 1.0) : 0.0;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    // scatterWeight /= (kernelSize * kernelSize);
    return shadow /= (kernelSize * kernelSize);

}

float computeShadow(LightUniform light, int lightId) {

    vec4 posLightSpace = light.viewProj * vec4(g_modelPos, 1.0);

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w; //For x,y and Z

    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.z < 0.0)
        return 0.0;

    vec3 lightDir = normalize(light.position.xyz - g_pos);
    float bias = max(light.shadowBias * 5.0 * (1.0 - dot(g_dir, lightDir)), light.shadowBias);  //Modulate by angle of incidence

    return filterPCF(lightId,int(light.pcfKernel), projCoords, bias);

}

float getLuminance(vec3 li) {
    return 0.2126 * li.r + 0.7152 * li.g + 0.0722 * li.b;
}

// vec3 multipleScattering(vec3 n){

//    vec3 l = normalize(u_scene.lightPos.xyz- g_pos);  //Light vector

//   float wrapLight = (dot(n,l)+1.0)/(4.0*PI);
//   vec3 scattering = sqrt(material.baseColor) * wrapLight * pow(material.baseColor/getLuminance(u_scene.lightColor),vec3(scatterWeight));
//   return scattering;

// }

// float unsharpSSAO(){ //Very simple SSAO with UNSHARP MASKING only using D BUFFER

//     const float kernelDimension = 15.0;
//     const ivec2 screenSize = textureSize(u_depthMap,0);

//     float occlusion = 0.0;

//     int i = int(gl_FragCoord.x);
//     int j = int(gl_FragCoord.y);

//     int maxX = i + int(floor(kernelDimension*0.5));
//     int maxY = j + int(floor(kernelDimension*0.5));

//     float sampX;
//     float sampY;

//     float neighborCount = 0;

//     for (int x = i - int(floor(kernelDimension*0.5)); x < maxX; x++) {
//     for (int y = j - int(floor(kernelDimension*0.5)); y < maxY; y++) {

//     sampX = float(x) / screenSize.x;
//     sampY = float(y) / screenSize.y;

//     if (sampX >= 0.0 && sampX <= 1.0 && sampY >= 0.0 && sampY <= 1.0 &&

//     abs( linearizeDepth(texture(u_depthMap,gl_FragCoord.xy / screenSize.xy).x, u_scene.frustrumData.x, u_scene.frustrumData.y) -
//      linearizeDepth(texture(u_depthMap,vec2(sampX,sampY)).x, u_scene.frustrumData.x, u_scene.frustrumData.y)) < 0.02) {
//     occlusion +=   linearizeDepth(texture(u_depthMap,vec2(sampX,sampY)).x, u_scene.frustrumData.x, u_scene.frustrumData.y);
//     neighborCount++;
//     }
//     }
//     }

//     occlusion = occlusion / neighborCount;

//     occlusion = 20 * ( linearizeDepth(texture(u_depthMap,gl_FragCoord.xy / screenSize.xy).x, u_scene.frustrumData.x, u_scene.frustrumData.y) - max(0.0, occlusion));

//   return clamp(occlusion,0.0,1.0);
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

    //Compute all lights
    vec3 color = vec3(0.0);
    for(int i = 0; i < scene.numLights; i++) {
        //If inside liught area influence
        if(isInAreaOfInfluence(scene.lights[i])) {

            vec3 lighting = computeLighting(material.roughness, material.shift, scene.lights[i], material.r, material.tt, material.trt);

            if(int(object.otherParams.y) == 1 && scene.lights[i].data.w == 1) {
                lighting *= (1.0 - computeShadow(scene.lights[i], i));

            }

            color += lighting;
        }
    }

    // if(u_scene.castShadow == 1.0) {
    //     color *= 1.0 - computeShadow();
    //     // if(material.useScatter && material.coloredScatter)
    //     //     color+= multipleScattering(n2);
    // }

    // vec3 n1 = cross(g_modelDir, cross(camera.position, g_modelDir));
    // vec3 n2 = normalize(g_modelPos-u_BVCenter);
    // vec3 fakeNormal = mix(n1,n2,0.5);
    vec3 fakeNormal = vec3(0.0);
    //Ambient component
    vec3 ambient = computeAmbient(fakeNormal);

    color += ambient;

    // if(material.occlusion){
    //   float occ = unsharpSSAO();
    //   color-=vec3(occ);
    // }

  // color = color / (color + vec3(1.0));
  //   const float GAMMA = 2.2;
  //   color = pow(color, vec3(1.0 / GAMMA));

    fragColor = vec4(color, 1.0);

}