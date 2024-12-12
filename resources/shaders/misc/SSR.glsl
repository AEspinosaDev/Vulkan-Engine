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



float distanceSquared(vec2 a, vec2 b) { a -= b; return dot(a, a); }

bool intersectsDepthBuffer(float rayZMin, float rayZMax, float sceneDepth, float thickness) {
    return (sceneDepth <= rayZMin) && (rayZMax <= (sceneDepth + thickness));
}
//Bynary Search
void refineTrace(float stride, int binaryRefinementSteps ,float thickness, vec3 csDir, inout vec3 hitPos, inout vec2 hitCoord) {

    // Either 1 or (-1) used to flip the stride
    float strideDirection = -1.0f;
    vec3 stride_vcs = stride * csDir;

    // Binary Search Refinement
    // If ssr.binaryRefinementSteps == 0, it simply skips loop and returns non-refined hit colour
    for (int step = 0; step < binaryRefinementSteps; ++step) {
        stride_vcs *= (strideDirection * 0.5f);
        vec3 mid_vcs = hitPos + stride_vcs;
        // vec4 mid_ccs = scene.WP * vec4(mid_vcs, 1.0f);
        vec4 mid_ccs = camera.unormProj * vec4(mid_vcs, 1.0f);
        vec2 mid_scs = mid_ccs.xy / mid_ccs.w;

        float depth = texelFetch(positionBuffer, ivec2(mid_scs), 0).w;

        if (depth == 0.0f) {
            // Sample texture out of bounds, skip this step
            continue;
        }

        float sceneDepth = lineariseDepth(depth);
        float midDepth = mid_ccs.w;

        if (intersectsDepthBuffer(midDepth, midDepth, sceneDepth,thickness)) {
            hitPos = mid_vcs;
            hitCoord = mid_scs;
            // Search in the opposite direction
            strideDirection *= -1.0f;
        }
    }
}
// Returns true if the ray hit something
bool traceScreenSpaceRay1(
 // Camera-space ray origin, which must be within the view volume
 vec3 csOrig, 
 // Unit length camera-space ray direction
 vec3 csDir,
 // The camera-space Z buffer (all negative values)
 sampler2D csZBuffer,
 // Camera space thickness to ascribe to each pixel in the depth buffer
 float zThickness, 
 // Step in horizontal or vertical pixels between samples. This is a float
 // because integer math is slow on GPUs, but should be set to an integer >= 1
 float stride,
 // Number between 0 and 1 for how far to bump the ray in stride units
 // to conceal banding artifacts
 float jitter,
 // Maximum number of iterations. Higher gives better images but may be slow
const uint maxSteps, 
 // Pixel coordinates of the first intersection with the scene
 out vec2 hitCoord, 
 // Camera space location of the ray hit
 out vec3 hitPoint) {

    // Clip to the near plane    
    float rayLength = ((csOrig.z + csDir.z * maxSteps) < camera.nearPlane) ?
        (camera.nearPlane - csOrig.z) / csDir.z : maxSteps;
    vec3 csEndPoint = csOrig + csDir * rayLength;

    // Project into homogeneous clip space
    vec4 H0 = camera.unormProj * vec4(csOrig, 1.0);
    vec4 H1 = camera.unormProj * vec4(csEndPoint, 1.0);
    float k0 = 1.0 / H0.w; 
    float k1 = 1.0 / H1.w;

    // The interpolated homogeneous version of the camera-space points  
    vec3 Q0 = vec3(csOrig.xy,-csOrig.z) * k0; 
    vec3 Q1 = csEndPoint * k1;

    // Screen-space endpoints
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;


    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += vec2((distanceSquared(P0, P1) < 0.01) ? 0.01 : 0.0);
    vec2 delta = P1 - P0;

    // Permute so that the primary iteration is in x to collapse
    // all quadrant-specific DDA cases later
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) {
        // This is a more-vertical line
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }

    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;

    // Track the derivatives of Q and k
    vec3  dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2  dP = vec2(stepDir, delta.y * invdx);

    // Construct PQk and dPQk
    vec4 PQk = vec4(P0, Q0.z, k0);
    vec4 dPQk = vec4(dP, dQ.z, dk);

    // Scale derivatives by the desired pixel stride and then
    // offset the starting values by the jitter fraction
    dPQk *= 1.0 + stride;
    PQk += 0.1 * dPQk;
   
    // Adjust end condition for iteration direction
    float end = P1.x * stepDir;

    // Sufficiently far away
    float prevZMaxEstimate = csOrig.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneDepth = rayZMax + camera.farPlane;

    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
    for(uint step = 0;
        ((PQk.x * stepDir) <= end) && (step < maxSteps) && (sceneDepth != 0.0f);
        ++step) {

        rayZMin = prevZMaxEstimate;
        rayZMax = (dPQk.z * 0.5f + PQk.z) / (dPQk.w * 0.5f + PQk.w);
        prevZMaxEstimate = rayZMax;

        if (rayZMin > rayZMax) {
            // Swap
            float temp = rayZMin;
            rayZMin = rayZMax;
            rayZMax = temp;
        }

        hitCoord = permute ? PQk.yx : PQk.xy;
        sceneDepth = lineariseDepth(texelFetch(positionBuffer, ivec2(hitCoord), 0).w);

        if (intersectsDepthBuffer(rayZMin, rayZMax, sceneDepth,zThickness)) {
            // Advance Q based on the number of steps
            vec3 Q = vec3(Q0.xy + dQ.xy * float(step), -PQk.z);
            hitPoint = Q * (1.0f / PQk.w);
            // stepsTaken = step;
            refineTrace(stride, BINARY_SEARCH_STEPS,zThickness,csDir, hitPoint, hitCoord);
            return true;
        }

        PQk += dPQk;
    }

    return false;
}
bool traceScreenSpaceRay2(
 // Camera-space ray origin, which must be within the view volume
 vec3 csOrig, 
 // Unit length camera-space ray direction
 vec3 csDir,
 // The camera-space Z buffer (all negative values)
 sampler2D csZBuffer,
 // Camera space thickness to ascribe to each pixel in the depth buffer
 float zThickness, 
 // Step in horizontal or vertical pixels between samples. This is a float
 // because integer math is slow on GPUs, but should be set to an integer >= 1
 float stride,
 // Number between 0 and 1 for how far to bump the ray in stride units
 // to conceal banding artifacts
 float jitter,
 // Maximum number of iterations. Higher gives better images but may be slow
const uint maxSteps, 
 // Pixel coordinates of the first intersection with the scene
 out vec2 hitCoord, 
 // Camera space location of the ray hit
 out vec3 hitPoint) {
    vec3 march_vcs = csOrig;
    vec3 stride_vcs = stride * csDir;
    float sceneDepth = camera.farPlane;

    for (uint step = 0; step < maxSteps && sceneDepth != 0.0f; ++step) {
        march_vcs += stride_vcs;
        vec4 march_ccs = camera.unormProj * vec4(march_vcs, 1.0f);
        vec2 march_scs = march_ccs.xy / march_ccs.w;

        float marchDepth = march_ccs.w;
        sceneDepth = lineariseDepth(texelFetch(positionBuffer, ivec2(march_scs), 0).a);

        if (intersectsDepthBuffer(marchDepth, marchDepth, sceneDepth,zThickness)) {
            hitPoint = march_vcs;
            hitCoord = march_scs;
            // stepsTaken = step;
             refineTrace(stride, BINARY_SEARCH_STEPS,zThickness,csDir, hitPoint, hitCoord);
            return true;
        }
    }

    return false;
 }
vec3 reconstructPositionVcs(vec2 uv, float depth) {
    // Compute NDC
    ivec2 screenSize = textureSize(positionBuffer, 0);

    // Clip-space coordinates (z = depth, w = 1.0)
    vec4 position_ccs = vec4(uv * 2.0f - 1.0f, depth, 1.0);

    // Reconstruct view-space position by multiplying with the inverse projection matrix
    vec4 position_vcs = camera.invProj * position_ccs;

    // Perform perspective division to get the final view-space position
    position_vcs /= position_vcs.w;

    return position_vcs.xyz;
}

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

    g_pos = reconstructPositionVcs(v_uv,g_depth);

    // if(g_metallic  < 0.01 && materialData.w != UNLIT_MATERIAL){
    //       outputImage = vec4(texture(inputImage,v_uv).rgb,1.0);
    //       return;
    // }

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, g_albedo, g_metallic);
    vec3 fresnel = fresnelSchlick(max(dot(g_normal, normalize(g_pos)), 0.0), F0);

    // Reflection vector
    vec3 R = normalize(reflect(normalize(g_pos), g_normal));


    //False Importance Sampling of BRDF Roughness
    vec3 modelPos = vec3(camera.invView * vec4(g_pos, 1.0));
    vec3 jitt = mix(vec3(0.0), vec3(hash0(modelPos,vec3(.8, .8, .8),19.19)), g_rough);
    
    //Raymarch through depth buffer
    vec3 hitPos = g_pos;
    vec2 hitCoord = vec2(0.0);
    bool hit = traceScreenSpaceRay2(g_pos, R+jitt, positionBuffer,0.5,0.1,0.9,64,hitCoord,hitPos);
    hitCoord = hit ? hitCoord / textureSize(positionBuffer, 0) : vec2(-1.0f);

    vec3 reflectionColour = hit ? texture(inputImage, hitCoord).rgb : vec3(0.0f);
    // bool isEnvironmentEnabled = (shadeUniforms.shade.detailsBitfield & environmentMapping) != 0;
    // vec3 environmentColour = !hit && isEnvironmentEnabled ? environmentColour(direction_vcs) : noReflection;
    
    // return reflectionColour + environmentColour;

    vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - hitCoord.xy));
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

     float reflectionPower = pow(g_metallic, REFLECTION_FALLOFF_EXP) * screenEdgefactor * -R.z;
    outputImage = vec4(texture(inputImage,v_uv).rgb + clamp(reflectionPower, 0.0, 0.9) * fresnel * reflectionColour,1.0);
    // outputImage = vec4(reflectionColour,1.0);
 
    // if (traceScreenSpaceRay1(g_pos,R,camera.proj,positionBuffer,0.005,-0.1,1,0.5,32,1000.0,hitCoords,hitPos)){
    // vec3 SSR = (texture(inputImage, hitCoords.xy).rgb * clamp(reflectionPower, 0.0, 0.9) * fresnel);  
    // }else
    //  outputImage = vec4(texture(inputImage,v_uv).rgb,1.0);
    // outputImage = vec4(SSR, 1.0);

}
