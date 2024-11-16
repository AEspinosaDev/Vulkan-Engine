#ifndef PI 
#define PI              3.1415926535897932384626433832795
#endif
#ifndef EPSILON 
#define EPSILON         0.001
#endif  

struct                  SchlickSmithBRDF{
    vec3                albedo;
    float               opacity;
    vec3                normal;
    float               metalness;
    float               roughness;
    float               ao;
    vec3                F0;
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

vec3 evalSchlickSmithBRDF(
    vec3 wi,
    vec3 wo, 
    vec3 radiance, 
    SchlickSmithBRDF brdf) 
    {

    //Vector setup
    vec3 h                  = normalize(wi + wo); 

	// Cook-Torrance BRDF
    float NDF               = distributionGGX(brdf.normal, h, brdf.roughness);
    float G                 = geometrySmith(brdf.normal, wo, wi, brdf.roughness);
    vec3 F                  = fresnelSchlick(max(dot(h, wo), 0.0), brdf.F0);

    vec3 kD                 = vec3(1.0) - F;
    kD                      *= 1.0 - brdf.metalness;

    vec3 numerator          = NDF * G * F;
    float denominator       = 4.0 * max(dot(brdf.normal, wo), 0.0) * max(dot(brdf.normal, wi), 0.0) + 0.0001;
    vec3 specular           = numerator / denominator;

	// Add to outgoing radiance result
    float lambertian        = max(dot(brdf.normal, wi), 0.0);
    
    return (kD * brdf.albedo / PI + specular) * radiance * lambertian;

}
