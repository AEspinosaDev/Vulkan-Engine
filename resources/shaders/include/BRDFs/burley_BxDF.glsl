//////////////////////////////////////////////
// ATTENTTION
// This script needs: fresnel.glsl
//////////////////////////////////////////////

#ifndef PI 
#define PI              3.1415926535897932384626433832795
#endif
#ifndef EPSILON 
#define EPSILON         0.001
#endif  

//Simplified Burley Function
struct                  BurleySimpleBRDF{
    vec3                albedo;
    float               opacity;
    vec3                normal;
    float               metalness;
    float               roughness;
    float               ao;
    vec3                emission;
    vec3                F0;
    vec3                F;
    float               diffuseModel;
};

//Complete Burley Function (2015 definition) accounting for scattering and other layers effects.
struct                  BurleyBSDF{
    vec3                albedo;
    float               opacity;
    vec3                normal;
    float               metalness;
    float               roughness;
    float               ao;
    vec3                emission;
    vec3                F0;
    vec3                F;

    //WIP
};

//Normal Distribution 
//Trowbridge - Reitz GGX
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a         = roughness * roughness;
    float a2        = a * a;
    float NdotH     = max(dot(N, H), 0.0);
    float NdotH2    = NdotH * NdotH;

    float num       = a2;
    float denom     = (NdotH2 * (a2 - 1.0) + 1.0);
    denom           = PI * denom * denom;

    return num / denom;
}
//Geometry
//Schlick - GGX
float geometrySchlickGGX(float NdotV, float roughness) {
    float r         = (roughness + 1.0);
    float k         = (r * r) / 8.0;

    float num       = NdotV;
    float denom     = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV     = max(dot(N, V), 0.0);
    float NdotL     = max(dot(N, L), 0.0);
    float ggx2      = geometrySchlickGGX(NdotV, roughness);
    float ggx1      = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

//Burley (Disney) diffuse term
vec3 diffuseBurley(vec3 wi, vec3 wo, vec3 n, vec3 albedo, float roughness)
{
    float NdotL = max(dot(n, wi), 0.0);
    float NdotV = max(dot(n, wo), 0.0);

    float FL = pow(1.0 - NdotL, 5.0);
    float FV = pow(1.0 - NdotV, 5.0);
    float Fd90 = 0.5 + 2.0 * roughness * NdotL * NdotL;

    float diffuseFactor = (1.0 + (Fd90 - 1.0) * FL) * (1.0 + (Fd90 - 1.0) * FV);
    return albedo * (diffuseFactor * NdotL / PI);
}

//Oren–Nayar diffuse term
vec3 diffuseOrenNayar(vec3 wi, vec3 wo, vec3 n, vec3 albedo, float roughness)
{
    float NdotL = max(dot(n, wi), 0.0);
    float NdotV = max(dot(n, wo), 0.0);

    if (NdotL <= 0.0 || NdotV <= 0.0)
        return vec3(0.0);

    float sigma = roughness * roughness;
    float sigma2 = sigma * sigma;

    float A = 1.0 - (sigma2 / (2.0 * (sigma2 + 0.33)));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    vec3 vi = normalize(wi - n * dot(n, wi));
    vec3 vo = normalize(wo - n * dot(n, wo));
    float cosPhiDiff = max(dot(vi, vo), 0.0);

    float alpha = max(acos(NdotL), acos(NdotV));
    float beta  = min(acos(NdotL), acos(NdotV));

    float orenNayar = NdotL * (A + B * cosPhiDiff * sin(alpha) * tan(beta));

    return albedo * orenNayar / PI;
}

// Dispatcher for diffuse type
vec3 getDiffuseTerm(vec3 wi, vec3 wo, BurleySimpleBRDF brdf)
{
    if (brdf.diffuseModel == 0) {
        // Lambert
        float NdotL = max(dot(brdf.normal, wi), 0.0);
        return brdf.albedo * NdotL / PI;
    } 
    else if (brdf.diffuseModel == 1) {
        // Burley
        return diffuseBurley(wi, wo, brdf.normal, brdf.albedo, brdf.roughness);
    } 
    else {
        // Oren–Nayar
        return diffuseOrenNayar(wi, wo, brdf.normal, brdf.albedo, brdf.roughness);
    }
}

vec3 evalBurleySimpleBRDF(
    vec3 wi,
    vec3 wo, 
    vec3 radiance, 
    BurleySimpleBRDF brdf) 
    {

    //Vector setup
    vec3 h                  = normalize(wi + wo); 

	// Cook-Torrance Specular Term
	// -----------------------------------
    float NDF               = distributionGGX(brdf.normal, h, brdf.roughness);
    float G                 = geometrySmith(brdf.normal, wo, wi, brdf.roughness);
    vec3 F                  = fresnelSchlick(max(dot(h, wo), 0.0), brdf.F0);

    vec3 kD                 = vec3(1.0) - F;
    kD                      *= 1.0 - brdf.metalness;

    vec3 numerator          = NDF * G * F;
    float denominator       = 4.0 * max(dot(brdf.normal, wo), 0.0) * max(dot(brdf.normal, wi), 0.0) + 0.0001;
    vec3 specular           = numerator / denominator;

    //Clamp specular value
    float roughnessThreshold    = 0.1;  
    float smoothness            = 1.0 - smoothstep(0.0, roughnessThreshold, brdf.roughness);
    float specularScale         = 1.0 - smoothness * 0.1;  // Scaling factor reduces specular at low roughness
    specular                    *= specularScale;

	// -----------------------------------
	// Diffuse Term
	// -----------------------------------
    
    vec3 diffuse = getDiffuseTerm(wi, wo, brdf);

	// -----------------------------------
    //Store fresnel value
	// -----------------------------------
    brdf.F = F;
    
    return (kD * diffuse + specular) * radiance;

}
// For IBL
vec3 evalDiffuseBurleySimpleBRDF(
    vec3 wi, 
    vec3 wo, 
    vec3 irradiance, 
    BurleySimpleBRDF brdf){

    float WidotWo = max(dot(wi, wo), 0.0);
    vec3 specularity = fresnelSchlickRoughness(WidotWo, brdf.F0, brdf.roughness);
    vec3 aDiffuse = vec3(1.0)  - specularity;
    aDiffuse *= 1.0 - brdf.metalness;	

    vec3 diffuse = irradiance * brdf.albedo;
    return aDiffuse * diffuse;
} 
vec3 evalSpecularBurleySimpleBRDF(
    vec3 wi, 
    vec3 wo, 
    vec3 irradiance, 
    sampler2D brdfLUT,
    BurleySimpleBRDF brdf){

    float WidotWo = max(dot(wi, wo), 0.0);
    vec3 specularity = fresnelSchlickRoughness(WidotWo, brdf.F0, brdf.roughness);
    // vec2 brdfSpec = texture(brdfLUT, vec2(WidotWo, brdf.roughness )).rg;
    // vec3 specular = irradiance * (specularity * brdfSpec.x + brdfSpec.y);
    vec3 specular = irradiance * specularity;
    return  specular;
} 
