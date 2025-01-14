#shader vertex
#version 460
#include camera.glsl

//INPUT
layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

//OUTPUT
layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);

    v_uv = uv;
}

#shader fragment
#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
#include material_defines.glsl
#include camera.glsl
#include light.glsl
#include scene.glsl
#include utils.glsl
#include shadow_mapping.glsl
#include fresnel.glsl
#include IBL.glsl
#include BRDFs/schlick_smith_BRDF.glsl
#include BRDFs/marschner_BSDF.glsl
#include warp.glsl
#include raytracing.glsl
#include hashing.glsl
#include SSR.glsl
#include VXGI.glsl


//INPUT
layout(location = 0) in  vec2 v_uv;

//OUTPUT
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBrightColor;

//UNIFORMS
layout(set = 0, binding =   2) uniform sampler2DArray              shadowMap;
layout(set = 0, binding =   4) uniform samplerCube                 irradianceMap;
layout(set = 0,  binding =  5) uniform accelerationStructureEXT    TLAS;
layout(set = 0,  binding =  6) uniform sampler2D                   blueNoiseMap;
layout(set = 0,  binding =  7) uniform sampler3D                   voxelMap;
//G-BUFFER
layout(set = 1, binding = 0) uniform sampler2D positionBuffer;
layout(set = 1, binding = 1) uniform sampler2D normalBuffer;
layout(set = 1, binding = 2) uniform sampler2D colorBuffer;
layout(set = 1, binding = 3) uniform sampler2D materialBuffer;
layout(set = 1, binding = 4) uniform sampler2D emissionBuffer;
layout(set = 1, binding = 5) uniform sampler2D preCompositionBuffer;

layout(set = 1, binding = 6) uniform sampler2D prevBuffer;

//SETTINGS
layout(push_constant) uniform Settings {
    uint    bufferOutput;
    uint    enableAO;
    VXGI    vxgi;   //Voxel Based GI
    SSR     ssr;    //Screen Space Reflections
} settings;

// DEBUGGING QUERIES
#define LIGHTING_MODE   settings.bufferOutput == 0
#define ALBEDO_OUTPUT   settings.bufferOutput == 1
#define NORMALS_OUTPUT  settings.bufferOutput == 2
#define POSITION_OUTPUT settings.bufferOutput == 3
#define MATERIAL_OUTPUT settings.bufferOutput == 4
#define SSAO_OUTPUT     settings.bufferOutput == 5

//SURFACE PROPERTIES
vec3    g_pos; 
float   g_depth; 
vec3    g_normal; 
vec3    g_albedo; 
float   g_opacity;
vec4    g_material; 
vec3    g_emission; 
int     g_isReflective;
vec4    g_temp; 



vec3 GI(vec3 worldPos, vec3 worldNormal){
    const float VOXEL_SIZE  = 1.0/float(settings.vxgi.resolution);
    const float ISQRT2 = 0.707106;
    const float ANGLE_MIX = 0.5; // Angle mix (1.0f => orthogonal direction, 0.0f => direction of normal).
	const float w[3] = {1.0, 1.0, 1.0}; // Cone weights.

	// Find a base for the side cones with the normal as one of its base vectors.
	const vec3 ortho = normalize(orthogonal(worldNormal));
	const vec3 ortho2 = normalize(cross(ortho, worldNormal));

	// Find base vectors for the corner cones too.
	const vec3 corner = 0.5 * (ortho + ortho2);
	const vec3 corner2 = 0.5 * (ortho - ortho2);

	// Find start position of trace (start with a bit of offset).
	const vec3 N_OFFSET = worldNormal * (1 + 4 * ISQRT2) * VOXEL_SIZE;
	const vec3 C_ORIGIN = worldPos + N_OFFSET;

	// Accumulate indirect diffuse light.
	vec3 indirect = vec3(0);

	// We offset forward in normal direction, and backward in cone direction.
	// Backward in cone direction improves GI, and forward direction removes
	// artifacts.
	const float CONE_OFFSET = -0.01;
    const float CONE_SPREAD = 0.325;

	// Trace front cone
	indirect += w[0] * traceVoxelCone(voxelMap, C_ORIGIN + CONE_OFFSET * worldNormal, worldNormal,CONE_SPREAD, VOXEL_SIZE );

	// Trace 4 side cones.
	const vec3 s1 = mix(worldNormal, ortho, ANGLE_MIX);
	indirect += w[1] * traceVoxelCone(voxelMap, C_ORIGIN + CONE_OFFSET * ortho, s1,CONE_SPREAD, VOXEL_SIZE );

	const vec3 s2 = mix(worldNormal, -ortho, ANGLE_MIX);
	indirect += w[1] * traceVoxelCone(voxelMap, C_ORIGIN - CONE_OFFSET * ortho, s2,CONE_SPREAD, VOXEL_SIZE );

	const vec3 s3 = mix(worldNormal, ortho2, ANGLE_MIX);
	indirect += w[1] * traceVoxelCone(voxelMap, C_ORIGIN + CONE_OFFSET * ortho2, s3,CONE_SPREAD, VOXEL_SIZE );

	const vec3 s4 = mix(worldNormal, -ortho2, ANGLE_MIX);
	indirect += w[1] * traceVoxelCone(voxelMap, C_ORIGIN - CONE_OFFSET * ortho2, s4,CONE_SPREAD, VOXEL_SIZE );

	// Trace 4 corner cones.
	const vec3 c1 = mix(worldNormal, corner, ANGLE_MIX);
	indirect += w[2] * traceVoxelCone(voxelMap, C_ORIGIN + CONE_OFFSET * corner, c1,CONE_SPREAD, VOXEL_SIZE );

	const vec3 c2 = mix(worldNormal, -corner, ANGLE_MIX);
	indirect += w[2] * traceVoxelCone(voxelMap, C_ORIGIN - CONE_OFFSET * corner, c2,CONE_SPREAD, VOXEL_SIZE );

	const vec3 c3 = mix(worldNormal, corner2, ANGLE_MIX);
	indirect += w[2] * traceVoxelCone(voxelMap, C_ORIGIN + CONE_OFFSET * corner2, c3,CONE_SPREAD, VOXEL_SIZE );

	const vec3 c4 = mix(worldNormal, -corner2, ANGLE_MIX);
	indirect += w[2] * traceVoxelCone(voxelMap, C_ORIGIN - CONE_OFFSET * corner2, c4,CONE_SPREAD, VOXEL_SIZE );

	// Return result.
	// return DIFFUSE_INDIRECT_FACTOR * material.diffuseReflectivity * acc * (material.diffuseColor + vec3(0.001f));
	return indirect;


}

void main()
{

    //////////////////////////////////////
    // SETUP SURFACE
    //////////////////////////////////////
    vec4 positionData = texture(positionBuffer,v_uv);
    g_pos       = positionData.rgb;
    g_depth     = positionData.w;
    g_normal    = normalize(texture(normalBuffer,v_uv).rgb);
    vec4 colorData = texture(colorBuffer,v_uv);
    g_albedo    = colorData.rgb;
    g_opacity   = colorData.w;
    g_material  = texture(materialBuffer,v_uv);
    vec4 emissionFresnelThreshold = texture(emissionBuffer,v_uv);
    g_emission  = emissionFresnelThreshold.rgb;
    g_isReflective = int(emissionFresnelThreshold.w);
    g_temp      = vec4(0.0);
    ///////////////////////////////////
    // PRE-COMPUTED DATA
    ///////////////////////////////////
    vec2 preComputedData    = texture(preCompositionBuffer,v_uv).rg;
    float SSAO              = preComputedData.r;
    float rtShadow          = preComputedData.g;


    vec3 color            = vec3(0.0);
    vec3 reflectedColor   = vec3(0.0);

    if(LIGHTING_MODE){
    //////////////////////////////////////
    // IF LIT MATERIAL
    //////////////////////////////////////
    if(g_material.w != UNLIT_MATERIAL){

        vec3 direct     = vec3(0.0);
        vec3 ambient    = vec3(0.0);
        vec3 indirect   = vec3(0.0);
        vec3 indirect2   = vec3(0.0);

        vec3 modelPos = (camera.invView * vec4(g_pos.xyz, 1.0)).xyz;
        vec3 modelNormal = (camera.invView  * vec4(g_normal.xyz, 0.0)).xyz;
        //////////////////////////////////////
        // PHYSICAL 
        //////////////////////////////////////
        if(g_material.w == PHYSICAL_MATERIAL){
            //Populate BRDF ________________________
            SchlickSmithBRDF brdf;
            brdf.albedo = g_albedo;
            brdf.opacity = g_opacity;
            brdf.normal = g_normal;
            brdf.roughness = g_material.x;
            brdf.metalness = g_material.y;
            brdf.ao = g_material.z;
            // brdf.emission;
            brdf.F0 = vec3(0.04);
            brdf.F0 = mix(brdf.F0, brdf.albedo, brdf.metalness);
            brdf.emission = g_emission;
            // Indirect Component ____________________________
            if(settings.vxgi.enabled == 1){
                vec3 diffuseIndirect = vec3(0.0);

                const float VOXEL_SIZE  = 1.0/float(settings.vxgi.resolution);


                 // Offset startPos to avoid self occlusion
                vec3 startPos = modelPos + modelNormal * VOXEL_SIZE;
    
                float coneTraceCount = 0.0;
                float cosSum = 0.0;
	            for (int i = 0; i < DIFFUSE_CONE_COUNT; ++i)
                {
	            	float cosTheta = dot(modelNormal, DIFFUSE_CONE_DIRECTIONS[i]);

                    if (cosTheta < 0.0)
                        continue;

                    coneTraceCount += 1.0;
	            	diffuseIndirect += traceVoxelCone(voxelMap, startPos,  DIFFUSE_CONE_DIRECTIONS[i],DIFFUSE_CONE_APERTURE , VOXEL_SIZE );
                }

	            diffuseIndirect /= coneTraceCount;
                // indirectContribution.a *= u_ambientOcclusionFactor;
    
	            diffuseIndirect.rgb *= g_albedo * settings.vxgi.strength;


                indirect = diffuseIndirect.rgb;


                // indirect = GI(modelPos, modelNormal)*settings.vxgi.strength*g_albedo;
                indirect*= settings.enableAO == 1 ? (brdf.ao * SSAO) : brdf.ao;

                // vec3 specularConeDirection = reflect(normalize(camera.position.xyz-modelPos), modelNormal);
                // vec3 specularIndirect = vec3(0.0);
    
    
                //  specularIndirect =   traceVoxelCone(voxelMap, startPos, specularConeDirection, max(brdf.roughness, 0.05) , VOXEL_SIZE );
                // // specularIndirect = castCone(startPos, specularConeDirection, max(roughness, MIN_SPECULAR_APERTURE), MAX_TRACE_DISTANCE, minLevel).rgb * specColor.rgb * u_indirectSpecularIntensity;
                // indirect2+=specularIndirect;
            }
            

            //Direct Component ________________________
            for(int i = 0; i < scene.numLights; i++) {
                    //If inside liught area influence
                    if(isInAreaOfInfluence(scene.lights[i].position, g_pos,scene.lights[i].areaEffect,int(scene.lights[i].type))){
                        //Direct Component ________________________
                        vec3 lighting = vec3(0.0);
                        lighting = evalSchlickSmithBRDF( 
                            scene.lights[i].type != DIRECTIONAL_LIGHT ? normalize(scene.lights[i].position - g_pos) : normalize(scene.lights[i].position.xyz), //wi
                            normalize(-g_pos),                                                                                           //wo
                            scene.lights[i].color * computeAttenuation(scene.lights[i].position, g_pos,scene.lights[i].areaEffect,int(scene.lights[i].type)) *  scene.lights[i].intensity,              //radiance
                            brdf
                            );
                        //Shadow Component ________________________
                        if(scene.lights[i].shadowCast == 1) {
                            if(scene.lights[i].shadowType == 0) //Classic
                                lighting *= computeShadow(shadowMap, scene.lights[i], i, modelPos);
                            if(scene.lights[i].shadowType == 1) //VSM   
                                lighting *= computeVarianceShadow(shadowMap, scene.lights[i], i, modelPos);
                            if(scene.lights[i].shadowType == 2) //Raytraced  
                                lighting *= computeRaytracedShadow(
                                    TLAS, 
                                    blueNoiseMap,
                                    modelPos, 
                                    scene.lights[i].type != DIRECTIONAL_LIGHT ? scene.lights[i].worldPosition.xyz - modelPos : scene.lights[i].shadowData.xyz,
                                    int(scene.lights[i].shadowData.w), 
                                    scene.lights[i].area, 
                                    0);
                        }
                    direct += lighting;
                    }
            }
            //Emission Component ________________________
            direct += brdf.emission;
            //Ambient Component ________________________
            if(scene.useIBL){
                ambient = computeAmbient(
                    irradianceMap,
                    scene.envRotation,
                    modelNormal,
                    normalize(camera.position.xyz-modelPos),
                    brdf.albedo,
                    brdf.F0,
                    brdf.metalness,
                    brdf.roughness,
                    scene.ambientIntensity);
            }else{
                ambient = (scene.ambientIntensity * scene.ambientColor) * brdf.albedo;
            }
            ambient *= settings.enableAO == 1 ? (brdf.ao * SSAO) : brdf.ao;
          
            //SSR ________________________________
            vec3 fresnel = fresnelSchlick(max(dot(g_normal, normalize(g_pos)), 0.0), brdf.F0);
            if(settings.ssr.enabled == 1 && g_isReflective == 1){
                // Reflection vector
                vec3 refl = normalize(reflect(normalize(g_pos), g_normal));

                //False Importance Sampling of BRDF Roughness
                vec3 modelPos   = vec3(camera.invView * vec4(g_pos, 1.0));
                vec3 jitt       = mix(vec3(0.0), vec3(hash0(modelPos,vec3(.8, .8, .8),19.19)), brdf.roughness);
                
                //Raymarch through depth buffer
                vec3 hitPos             = g_pos; vec2 hitCoord = vec2(0.0);
                bool hit                = raymarchVCS(settings.ssr, positionBuffer, g_pos, refl, hitCoord, hitPos);
                hitCoord                = hit ? hitCoord / textureSize(positionBuffer, 0) : vec2(-1.0f);
                // hitCoord = clamp(hitCoord,vec2(0,0),vec2(1.0));
                vec3 reflectionColour   = hit ? texture(prevBuffer, hitCoord).rgb : vec3(0.0f);

                //Control edges
                vec2 dCoords            = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - hitCoord.xy));
                float screenEdgefactor  = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
                vec3 reflectionPower    = pow(brdf.metalness, REFLECTION_FALLOFF_EXP) * screenEdgefactor * -refl.z * fresnel;
                // vec3 reflectionPower    = fresnel * -refl.z* screenEdgefactor;
                
                // mat3 rotY = rotationY(radians(scene.envRotation));
                // reflectionColour += !hit && scene.useIBL ?  texture(irradianceMap, normalize(rotY*(camera.invView*vec4(refl,0.0)).xyz)).rgb*scene.ambientIntensity : vec3(0.0);

                reflectedColor = clamp(reflectionPower, 0.0, 0.9) * reflectionColour;
                // direct =  reflectionColour;
                // direct = vec3(hitCoord,hit);
            }

        }
        //////////////////////////////////////
        // PHONG 
        //////////////////////////////////////
        if(g_material.w == PHONG_MATERIAL){
        
            //TBD ....
        }
        //////////////////////////////////////
        // HAIR 
        //////////////////////////////////////
        if(g_material.w == HAIR_STRAND_MATERIAL){
            //TBD ....
        }

               
                    

    color = direct + indirect + ambient + reflectedColor;
      
    //////////////////////////////////////
    // IF UNLIT MATERIAL
    //////////////////////////////////////
    }else{
        color = g_albedo;
    }
    
    //Fog ________________________________
    if(scene.enableFog){
        float f = computeFog(g_depth);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }

    outColor = vec4(color,1.0);


    }
    else{ //DEBUG MODE
        if(ALBEDO_OUTPUT) 
            outColor = vec4(g_albedo,1.0);
        if(NORMALS_OUTPUT)
            outColor = vec4((camera.invView  * vec4(g_normal.xyz, 0.0)).xyz,1.0);
        if(POSITION_OUTPUT)
            outColor = vec4((camera.invView * vec4(g_pos.xyz, 1.0)).xyz,1.0);
        if(MATERIAL_OUTPUT)
            outColor = g_material;
        if(SSAO_OUTPUT) 
            outColor = vec4(SSAO,SSAO,SSAO,1.0);
    }

    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0 && LIGHTING_MODE)
        outBrightColor = vec4(color, 1.0);
    else
        outBrightColor = vec4(0.0, 0.0, 0.0, 1.0);

}

