vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  
vec3 computeAmbient(samplerCube irradianceMap,vec3 worldNormal, vec3 camPos, vec3 albedo, vec3 F0, float metalness, float roughness, float intensity){

    vec3 specularity = fresnelSchlickRoughness(max(dot(worldNormal, camPos), 0.0), F0,roughness);
    vec3 aDiffuse = vec3(1.0)  - specularity;
    aDiffuse *= 1.0 - metalness;	
    vec3 irradiance = texture(irradianceMap, worldNormal).rgb*intensity;
   
    vec3 diffuse = irradiance * albedo;
    return aDiffuse * diffuse;
} 
