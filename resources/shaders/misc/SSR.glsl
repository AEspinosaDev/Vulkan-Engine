#shader vertex
#version 460

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;


layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);
    v_uv = uv;
}

#shader fragment
#version 460
#include camera.glsl
#include fresnel.glsl
#include hashing.glsl
#include material_defines.glsl

layout(location = 0) in  vec2 v_uv;

layout(set = 0, binding = 2) uniform sampler2D inputImage;       //F32

layout(set = 1, binding = 0) uniform sampler2D positionBuffer;  //F32
layout(set = 1, binding = 1) uniform sampler2D normalBuffer;    //F32
layout(set = 1, binding = 2) uniform sampler2D colorBuffer;     //F32
layout(set = 1, binding = 3) uniform sampler2D materialBuffer;  //U8

layout(location = 0) out vec4 outputImage;

layout(push_constant) uniform Settings {
    float bloomStrenght; 
} settings;

#define STEP                    0.1
#define MIN_STEP_LENGTH         0.1
#define MAX_STEPS               30
#define BINARY_SEARCH_STEPS     5
#define REFLECTION_FALLOFF_EXP  3.0

//SURFACE PROPERTIES
vec3    g_pos; 
float   g_depth;    
vec3    g_normal; 
vec3    g_albedo;
float   g_metallic; 
float   g_rough; 

vec3 getPos(float depth);
vec3 binarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth);
vec4 raymarch(vec3 dir, inout vec3 hitCoord, out float dDepth);


void main()
{
    /*View-Space*/
    vec4 positionData = texture(positionBuffer,v_uv);
    g_pos       = positionData.rgb;
    g_depth     = positionData.w;
    g_normal    = normalize(texture(normalBuffer,v_uv).rgb);
    vec4 colorData = texture(colorBuffer,v_uv);
    g_albedo    = colorData.rgb;
    vec4 materialData = texture(materialBuffer, v_uv);
    g_metallic  = materialData.g;
    g_rough     = materialData.r;

    if(g_metallic  < 0.01 && materialData.w != UNLIT_MATERIAL){
          outputImage = vec4(texture(inputImage,v_uv).rgb,1.0);
          return;
    }

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, g_albedo, g_metallic);
    vec3 fresnel = fresnelSchlick(max(dot(g_normal, normalize(g_pos)), 0.0), F0);

    // Reflection vector
    vec3 R = normalize(reflect(normalize(g_pos), g_normal));

    vec3 hitPos = g_pos;
    float currentDepth;

    //False Importance Sampling of BRDF Roughness
    vec3 modelPos = vec3(camera.invView * vec4(g_pos, 1.0));
    vec3 jitt = mix(vec3(0.0), vec3(hash0(modelPos,vec3(.8, .8, .8),19.19)), 1.0-g_rough);
    //Raymarch through depth buffer
    vec4 coords = raymarch((vec3(jitt) + R * max(MIN_STEP_LENGTH, -g_pos.z)), hitPos, currentDepth);
 
    // Control screen edges
    vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

    float reflectionPower = pow(g_metallic, REFLECTION_FALLOFF_EXP) * screenEdgefactor * -R.z;
 
    // Reflected color
    vec3 SSR = (texture(inputImage, coords.xy).rgb * clamp(reflectionPower, 0.0, 0.9) * fresnel);  

    outputImage = vec4(mix(texture(inputImage,v_uv).rgb,SSR, g_metallic), 1.0);

}

vec3 getPos(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(v_uv * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = camera.invProj * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

vec3 binarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
    float depth;
    vec4 projectedCoord;
 
    for(int i = 0; i < BINARY_SEARCH_STEPS; i++)
    {

        projectedCoord = camera.proj * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        depth = texture(positionBuffer, projectedCoord.xy).z;
 
        dDepth = hitCoord.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
            hitCoord += dir;
        else
            hitCoord -= dir;    
    }

    projectedCoord = camera.proj * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
    return vec3(projectedCoord.xy, depth);
}

vec4 raymarch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{

    dir *= STEP;
 
    float depth;
    int steps;
    vec4 projectedCoord;
 
    for(int i = 0; i < MAX_STEPS; i++)
    {
        hitCoord += dir;
 
        projectedCoord = camera.proj  * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
        // if(projectedCoord.x > 1.0 || projectedCoord.y > 1.0 || projectedCoord.x < 0.0 || projectedCoord.y < 0.0) return vec4(0.0);
        depth = texture(positionBuffer, projectedCoord.xy).z;
        if(depth >= 100.0)
            continue;
 
        dDepth = hitCoord.z - depth;

        if((dir.z - dDepth) < 1.2)
        {
            if(dDepth <= 0.0)
            {   
                vec4 result;
                result = vec4(binarySearch(dir, hitCoord, dDepth), 1.0);

                return result;
            }
        }
        
        steps++;
    }
 
    
    return vec4(projectedCoord.xy, depth, 0.0);
}