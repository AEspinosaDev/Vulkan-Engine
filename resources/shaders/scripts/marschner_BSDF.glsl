/////////////////////////////////////////////

//EPIC'S UNREAL REAL-TIME ARTIST-FRIENDLY FIT
/////////////////////////////////////////////

#define R_NORMALIZING_FACTOR 1.5
#define TT_NORMALIZING_FACTOR 1.0
#define TRT_NORMALIZING_FACTOR 0.01

//Constant
const float PI = 3.14159265359;


struct MarschnerBSDF{
    vec3 tangent;

    vec3 baseColor;
    float beta;
    float shift;
    float ior;
    float Rpower;
    float TTpower;
    float TRTpower;
};


///Fresnel
float fresnelSchlick(float ior, float cosTheta) {
    float F0 = ((1.0 - ior) * (1.0 - ior)) / ((1.0 + ior) * (1.0 + ior));
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Azimuthal REFLECTION
float NR(vec3 wi, vec3 wo, float cosPhi, float ior) {
    float cosHalfPhi = sqrt(0.5 + 0.5 * cosPhi);

  // return (0.25*cosHalfPhi)*fresnelSchlick(bsdf.ior,sqrt(0.5*(1+dot(wi,wo)))); //Frostbite
    return (0.25 * cosHalfPhi) * fresnelSchlick(ior, sqrt(0.5 + 0.5 * dot(wi, wo))); //Epic
}
//Attenuattion
vec3 A(float f, int p, vec3 t) { //fresnel, Stage, Absorbtion
    return (1 - f) * (1 - f) * pow(f, p - 1) * pow(t, vec3(p));
}

// Azimuthal TRANSMITION
vec3 NTT(float sinThetaD, float cosThetaD, float cosPhi, float ior, vec3 absColor) {

  // float _ior = sqrt(bsdf.ior*bsdf.ior - sinThetaD* sinThetaD)/ cosThetaD; //Original
    float _ior = 1.19 / cosThetaD + 0.36 * cosThetaD; //Fit EPIC
    float a = 1 / _ior;

    float cosHalfPhi = sqrt(0.5 + 0.5 * cosPhi);
    float h = 1 + a * (0.6 - 0.8 * cosPhi) * cosHalfPhi; //Fit EPIC

    float power = sqrt(1 - h * h * a * a) / (2 * cosThetaD);

    float D = exp(-3.65 * cosPhi - 3.98); //Intensity distribution
    vec3 T = pow(absColor, vec3(power)); //Absortion
    float F = fresnelSchlick(ior, cosThetaD * sqrt(1 - h * h)); //Fresnel

    return A(F, 1, T) * D;
}
// Azimuthal DOUBLE REFLECTION
vec3 NTRT(float sinThetaD, float cosThetaD, float cosPhi,float ior, vec3 absColor) {
    float h = sqrt(3.0) * 0.5; //Por que si

    float D = exp(17.0 * cosPhi - 16.78); //Intensity distribution
    vec3 T = pow(absColor, vec3(0.8 / cosThetaD));
    float F = fresnelSchlick(ior, acos(cosThetaD * sqrt(1 - h * h))); //Fresnel CONSTANT ?

    return A(F, 2, T) * D;
}

//Longitudinal TERM
float M(float sinTheta, float roughness) {
  // return exp(-(sinTheta*sinTheta)/(2*roughness*roughness))/sqrt(2*PI*roughness); //Frostbite 
    return (1.0 / (roughness * sqrt(2 * PI))) * exp((-sinTheta * sinTheta) / (2.0 * roughness * roughness)); //Epic. sintheta = sinThetaWi+sinThetaV-alpha
}

vec3 evalMarschnerBSDF(
    vec3 wi,               //Light vector
    vec3 v,                //View vector
    vec3 irradiance,
    MarschnerBSDF bsdf,
    bool r,
    bool tt, 
    bool trt) 
    {


  //Betas
    float betaR = bsdf.beta * bsdf.beta;
    float betaTT = 0.5 * betaR;
    float betaTRT = 2.0 * betaR;


  //Theta & Phi
    vec3 u = bsdf.tangent;
    float sinThetaWi = dot(wi, u);
    float sinThetaV = dot(v, u);
    vec3 wiPerp = wi - sinThetaWi * u;
    vec3 vPerp = v - sinThetaV * u;
    float cosPhiD = dot(wiPerp, vPerp) / sqrt(dot(wiPerp, wiPerp) * dot(vPerp, vPerp));

  // Diff
    float thetaD = (asin(sinThetaWi) - asin(sinThetaV)) * 0.5;
    float cosThetaD = cos(thetaD);
    float sinThetaD = sin(thetaD);

    float R = r ? M(sinThetaWi + sinThetaV - bsdf.shift * 2.0, betaR) * NR(wi, v, cosPhiD, bsdf.ior) / 0.25 : 0.0;
    vec3 TT = tt ? M(sinThetaWi + sinThetaV + bsdf.shift, betaTT) * NTT(sinThetaD, cosThetaD, cosPhiD, bsdf.ior, bsdf.baseColor) : vec3(0.0);
    vec3 TRT = trt ? M(sinThetaWi + sinThetaV + bsdf.shift * 4.0, betaTRT) * (NTRT(sinThetaD, cosThetaD, cosPhiD,bsdf.ior, bsdf.baseColor)/TRT_NORMALIZING_FACTOR) : vec3(0.0);

    vec3 albedo = bsdf.baseColor;
    vec3 specular = (R * bsdf.Rpower + TT * bsdf.TTpower + TRT * bsdf.TRTpower) / max(0.2, cosThetaD * cosThetaD);

    return (specular + albedo) * irradiance;

}