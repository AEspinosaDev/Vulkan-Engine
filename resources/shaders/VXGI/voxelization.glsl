#shader vertex
#version 460

#include camera.glsl
#include object.glsl
#include light.glsl
#include scene.glsl
#include utils.glsl

//Input
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

//Output
layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec4 slot1; 
    vec4 slot2; 
    vec4 slot3; 
    vec4 slot4; 
    vec4 slot5; 
    vec4 slot6; 
    vec4 slot7; 
    vec4 slot8; 
} material;




void main() {

    v_pos = (object.model * vec4(pos, 1.0)).xyz;
    v_uv = vec2(uv.x * material.slot2.x, (1-uv.y) * material.slot2.y); 
    v_normal = normalize(mat3(transpose(inverse(object.model))) * normal);

    vec3 ndc = mapToZeroOne(v_pos, scene.minCoord.xyz, scene.maxCoord.xyz) * 2.0 -1.0;
    gl_Position = vec4(ndc, 1.0);
    
}
#shader geometry
#version 460
#extension GL_NV_geometry_shader_passthrough : enable


layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 v_pos[];
layout(location = 1) in vec3 v_normal[];
layout(location = 2) in vec2 v_uv[];

layout(location = 0) out vec3 _pos;
layout(location = 1) out vec3 _normal;
layout(location = 2) out vec2 _uv;

void main(){

    const vec3 p1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    const vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    const vec3 p = abs(cross(p1, p2));

	for(uint i = 0; i < 3; ++i){
		_pos = v_pos[i];
		_normal = v_normal[i];
        _uv = v_uv[i];
		if(p.z > p.x && p.z > p.y){
			gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, 0, 1);
		} else if (p.x > p.y && p.x > p.z){
			gl_Position = vec4(gl_in[i].gl_Position.y, gl_in[i].gl_Position.z, 0, 1);
		} else {
			gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.z, 0, 1);
		}
		EmitVertex();
	}
    EndPrimitive();
}


#shader fragment
#version 460

#define USE_IMG_ATOMIC_OPERATION

#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

#include light.glsl
#include scene.glsl
#include utils.glsl
#include shadow_mapping.glsl
#include warp.glsl
#include raytracing.glsl
#include material_defines.glsl


layout(location = 0) in vec3 _pos;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _uv;

layout(set = 0, binding =   2) uniform sampler2DArray              shadowMap;
layout(set = 0, binding =   3) uniform samplerCube                 irradianceMap;
layout(set = 0,  binding =  4) uniform accelerationStructureEXT    TLAS;
layout(set = 0,  binding =  5) uniform sampler2D                   samplerMap;

layout(set = 0,  binding =  6, r32f) uniform image3D               voxelImage;
#ifdef USE_IMG_ATOMIC_OPERATION
layout(set = 0,  binding =  7, r32ui) uniform uimage3D              auxVoxelImages[3];
#endif

layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec4 slot1; 
    vec4 slot2; 
    vec4 slot3; 
    vec4 slot4; 
    vec4 slot5; 
    vec4 slot6; 
    vec4 slot7; 
    vec4 slot8; 
} material;


layout(set = 2, binding = 0) uniform sampler2D albedoTex;
layout(set = 2, binding = 1) uniform sampler2D normalTex;
layout(set = 2, binding = 2) uniform sampler2D materialText1;
layout(set = 2, binding = 3) uniform sampler2D materialText2;
layout(set = 2, binding = 4) uniform sampler2D materialText3;
layout(set = 2, binding = 5) uniform sampler2D materialText4;

///////////////////////////////////////////
//Surface Global properties
///////////////////////////////////////////
vec3    g_albedo            = vec3(0.0);
float   g_opacity           = 1.0;
vec3    g_emisison          = vec3(0.0);

void setupSurfaceProperties(){

    //Depending on material
    if(material.slot8.w == PHYSICAL_MATERIAL){

        //Setting input surface properties
        g_albedo = int(material.slot4.w)== 1 ? mix(material.slot1.rgb, texture(albedoTex, _uv).rgb, material.slot3.x) : material.slot1.rgb;
        g_opacity = int(material.slot4.w)== 1 ?  mix(material.slot1.w, texture(albedoTex, _uv).a, material.slot6.z) : material.slot1.w;
        g_emisison = material.slot6.w == 1 ? mix(material.slot7.rgb, texture(materialText4, _uv).rgb, material.slot7.w) : material.slot7.rgb;
        g_emisison *= material.slot8.x;

    }
    if(material.slot8.w == UNLIT_MATERIAL){
        g_albedo = int(material.slot2.w) == 1 ? texture(albedoTex, _uv).rgb : material.slot1.rgb;
    }
    if(material.slot8.w == HAIR_STRAND_MATERIAL){
        // TBD .........
    }
    if(material.slot8.w == PHONG_MATERIAL){
        // TBD .........
    }
    

}
ivec3 worldSpaceToVoxelSpace(vec3 worldPos)
{
    vec3 uvw = mapToZeroOne(worldPos, scene.minCoord.xyz, scene.maxCoord.xyz);
    ivec3 voxelPos = ivec3(uvw * imageSize(voxelImage));
    return voxelPos;
}
vec3 evalDiffuseLighting(vec3 wi,
                        vec3 radiance)
{
    float cosTheta = max(dot(_normal, wi),0.0);
    if (cosTheta > 0.0)
    {
        vec3 diffuse = radiance * cosTheta * g_albedo;
        return diffuse;
    }
    return vec3(0.0);
}


void main() {
    setupSurfaceProperties();

    vec3 color = vec3(0.0);

    for(int i = 0; i < scene.numLights; i++) {
        if(isInAreaOfInfluence(scene.lights[i].worldPosition.xyz, _pos,scene.lights[i].areaEffect,int(scene.lights[i].type))){

            //Diffuse Component ________________________
            vec3 lighting = vec3(0.0);
            lighting = evalDiffuseLighting( 
                scene.lights[i].type != DIRECTIONAL_LIGHT ? normalize(scene.lights[i].worldPosition.xyz - _pos) : normalize(scene.lights[i].worldPosition.xyz), //wi                                                                                          //wo
                scene.lights[i].color * computeAttenuation(scene.lights[i].worldPosition.xyz, _pos,scene.lights[i].areaEffect,int(scene.lights[i].type)) *  scene.lights[i].intensity             
                );

            //Visibility Component__________________________
            if(scene.lights[i].shadowCast == 1) {
                        // if(scene.lights[i].shadowType == 0) //Classic
                        //     lighting *= computeShadow(shadowMap, scene.lights[i], i, _pos);
                        // if(scene.lights[i].shadowType == 1) //VSM   
                        //     lighting *= computeVarianceShadow(shadowMap, scene.lights[i], i, _pos);
                        if(scene.lights[i].shadowType == 2) //Raytraced  
                            lighting *= computeRaytracedShadow(
                                TLAS, 
                                samplerMap,
                                _pos, 
                                scene.lights[i].type != DIRECTIONAL_LIGHT ? scene.lights[i].worldPosition.xyz - _pos : scene.lights[i].shadowData.xyz,
                                1, 
                                0.0, 
                                0);
                    }
            
            color += lighting;
        }
    }

    color += g_emisison;

    vec3 ambient = scene.ambientIntensity * g_albedo;
    color +=ambient;

	vec4 result = g_opacity * vec4(vec3(color), 1);
    ivec3 voxelPos = worldSpaceToVoxelSpace(_pos);

#ifdef USE_IMG_ATOMIC_OPERATION
    imageAtomicMax(auxVoxelImages[0], voxelPos, floatBitsToUint(result.r));
    imageAtomicMax(auxVoxelImages[1], voxelPos, floatBitsToUint(result.g));
    imageAtomicMax(auxVoxelImages[2], voxelPos, floatBitsToUint(result.b));
    imageStore(voxelImage, voxelPos, vec4(0.0, 0.0, 0.0, 1.0));
#else
    imageStore(voxelImage, voxelPos, result);
#endif

}
