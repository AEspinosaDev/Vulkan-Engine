/////////////////////////////////////////////
// LUT BASED
/////////////////////////////////////////////

//ConstantS
const float PI =                3.14159265359;


struct                          MarschnerLookupBSDF{
    vec3                        tangent;
    vec3                        baseColor;
    float                       Rpower;
    float                       TTpower;
    float                       TRTpower;
};

//Longitudinal TERM
float M(float sinTheta, float roughness) {
  // return exp(-(sinTheta*sinTheta)/(2*roughness*roughness))/sqrt(2*PI*roughness); //Frostbite 
    return (1.0 / (roughness * sqrt(2 * PI))) * exp((-sinTheta * sinTheta) / (2.0 * roughness * roughness)); //Epic. sintheta = sinThetaWi+sinThetaV-alpha
}

vec3 evalMarschnerLookupBSDF(
    vec3 wi,               //Light vector
    vec3 v,                //View vector
    vec3 irradiance,
    MarschnerLookupBSDF bsdf,
    sampler2D texM,
    sampler2D texN,
    bool r,
    bool tt, 
    bool trt) 
    {

    //Theta & Phi
    vec3 u            = bsdf.tangent;
    float sin_thI     = dot(wi, u);
    float sin_thR     = dot(v, u);
    vec3 wiPerp       = normalize(wi - sin_thI * u);
    vec3 vPerp        = normalize(v - sin_thR * u);
    float cos_phiD    = dot(vPerp,wiPerp)*inversesqrt(dot(vPerp,vPerp)*dot(wiPerp,wiPerp));
    float cos_thD     = cos((asin(sin_thI)-asin(sin_thR))*0.5);

    //Look ups
    vec2 uv_M         = vec2(sin_thI,sin_thR)*0.5+0.5;
    vec4 M            = texture(texM,uv_M).rgba; 
    // float cos_thD  = mM.a; 
    vec2 uv_N         = vec2(cos_phiD,cos_thD)*0.5+0.5;
    vec4 N            = texture(texN,uv_N).rgba;

    float R           = r ?   M.r * N.r : 0.0; 
    float TT          = tt ?  M.g * N.g : 0.0; 
    float TRT         = trt ? M.b * N.b : 0.0; 

    vec3 albedo       = bsdf.baseColor;
    vec3 specular     = vec3(R * bsdf.Rpower + TT * bsdf.TTpower + TRT * bsdf.TRTpower) / max(0.2, cos_thD * cos_thD);

    return (specular + albedo) * irradiance;

}