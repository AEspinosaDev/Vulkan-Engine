//////////////////////////////////////////////
// ATTENTTION
// This script needs: camera.glsl
//////////////////////////////////////////////

/*
SCREEN SPACE REFLECTIONS RAYMARCHING ALGORYTHMS
*/

struct SSR {
    uint maxSteps;
    float stride;
    uint binaryRefinementSteps;
    float thickness;
    float jitter;
    uint enabled;
};

#define REFLECTION_FALLOFF_EXP  3.0

float distanceSquared(vec2 a, vec2 b) {
    a -= b;
    return dot(a, a);
}

bool intersectsDepthBuffer(float rayZMin, float rayZMax, float sceneDepth, float thickness) {
    return (sceneDepth <= rayZMin) && (rayZMax <= (sceneDepth + thickness));
}

//Bynary Search
void refineTrace(SSR ssr, sampler2D vPosBuffer, vec3 vDir, inout vec3 hitPos, inout vec2 hitCoord) {

    // Either 1 or (-1) used to flip the stride
    float strideDirection = -1.0;
    vec3 stride_vcs = ssr.stride * vDir;

    // Binary Search Refinement
    // If ssr.binaryRefinementSteps == 0, it simply skips loop and returns non-refined hit colour
    for(int step = 0; step < ssr.binaryRefinementSteps; ++step) {
        stride_vcs *= (strideDirection * 0.5);
        vec3 mid_vcs = hitPos + stride_vcs;
        // vec4 mid_ccs = scene.WP * vec4(mid_vcs, 1.0f);
        vec4 mid_ccs = camera.unormProj * vec4(mid_vcs, 1.0);
        vec2 mid_scs = mid_ccs.xy / mid_ccs.w;

        float depth = texelFetch(vPosBuffer, ivec2(mid_scs), 0).a;

        if(depth == 0.0) {
            // Sample texture out of bounds, skip this step
            continue;
        }

        float sceneDepth = lineariseDepth(depth);
        float midDepth = mid_ccs.w;

        if(intersectsDepthBuffer(midDepth, midDepth, sceneDepth, ssr.thickness)) {
            hitPos = mid_vcs;
            hitCoord = mid_scs;
            // Search in the opposite direction
            strideDirection *= -1.0;
        }
    }
}
// Based on the work of Morgan McGuire and Michael Mara at Williams College 2014:
// https://www.jcgt.org/published/0003/04/04/
//
// Released as open source under the BSD 2-Clause License
// http://opensource.org/licenses/BSD-2-Clause
//
// Copyright (c) 2014, Morgan McGuire and Michael Mara
// All rights reserved.
//
// From McGuire and Mara, Efficient GPU Screen-Space Ray Tracing,
// Journal of Computer Graphics Techniques, 2014
bool raymarchDDA(SSR ssr, sampler2D vPosBuffer, vec3 csOrig, vec3 csDir, out vec2 hitCoord, out vec3 hitPoint) {

    // Clip to the near plane    
    float rayLength = ((csOrig.z + csDir.z * ssr.maxSteps) < camera.nearPlane) ? (camera.nearPlane - csOrig.z) / csDir.z : ssr.maxSteps;
    vec3 csEndPoint = csOrig + csDir * rayLength;

    // Project into homogeneous clip space
    vec4 H0 = camera.unormProj * vec4(csOrig, 1.0);
    vec4 H1 = camera.unormProj * vec4(csEndPoint, 1.0);
    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    // The interpolated homogeneous version of the camera-space points  
    vec3 Q0 = vec3(csOrig.xy, -csOrig.z) * k0;
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
    if(abs(delta.x) < abs(delta.y)) {
        // This is a more-vertical line
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }

    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;

    // Track the derivatives of Q and k
    vec3 dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2 dP = vec2(stepDir, delta.y * invdx);

    // Construct PQk and dPQk
    vec4 PQk = vec4(P0, Q0.z, k0);
    vec4 dPQk = vec4(dP, dQ.z, dk);

    // Scale derivatives by the desired pixel stride and then
    // offset the starting values by the jitter fraction
    dPQk *= 1.0 + ssr.stride;

    PQk += ssr.jitter * dPQk;

    // Adjust end condition for iteration direction
    float end = P1.x * stepDir;

    // Sufficiently far away
    float prevZMaxEstimate = csOrig.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneDepth = rayZMax + camera.farPlane;

    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
    for(uint step = 0; ((PQk.x * stepDir) <= end) && (step < ssr.maxSteps) && (sceneDepth != 0.0); ++step) {

        rayZMin = prevZMaxEstimate;
        rayZMax = (dPQk.z * 0.5 + PQk.z) / (dPQk.w * 0.5 + PQk.w);
        prevZMaxEstimate = rayZMax;

        if(rayZMin > rayZMax) {
            // Swap
            float temp = rayZMin;
            rayZMin = rayZMax;
            rayZMax = temp;
        }

        hitCoord = permute ? PQk.yx : PQk.xy;
        sceneDepth = lineariseDepth(texelFetch(vPosBuffer, ivec2(hitCoord), 0).w);

        if(intersectsDepthBuffer(rayZMin, rayZMax, sceneDepth, ssr.thickness)) {
            // Advance Q based on the number of steps
            vec3 Q = vec3(Q0.xy + dQ.xy * float(step), -PQk.z);
            hitPoint = Q * (1.0 / PQk.w);
            // stepsTaken = step;
            refineTrace(ssr, vPosBuffer, csDir, hitPoint, hitCoord);
            return true;
        }

        PQk += dPQk;
    }

    return false;
}
bool raymarchVCS(SSR ssr, sampler2D vPosBuffer, vec3 vOrig, vec3 vDir, out vec2 hitCoord, out vec3 hitPoint) {

    vec3 march_vcs = vOrig;
    vec3 stride_vcs = ssr.stride * vDir;
    float sceneDepth = camera.farPlane;

    for(uint step = 0; step < ssr.maxSteps && sceneDepth != 0.0; ++step) {
        march_vcs += stride_vcs;
        vec4 march_ccs = camera.unormProj * vec4(march_vcs, 1.0);
        vec2 march_scs = march_ccs.xy / march_ccs.w;

        float marchDepth = march_ccs.w;
        sceneDepth = lineariseDepth(texelFetch(vPosBuffer, ivec2(march_scs), 0).a);

        if(intersectsDepthBuffer(marchDepth, marchDepth, sceneDepth, ssr.thickness)) {
            hitPoint = march_vcs;
            hitCoord = march_scs;
            // stepsTaken = step;
            refineTrace(ssr, vPosBuffer, vDir, hitPoint, hitCoord);
            return true;
        }
    }

    return false;
}
vec3 reconstructPositionVcs(sampler2D vPosBuffer, vec2 uv, float depth) {
    // Compute NDC
    ivec2 screenSize = textureSize(vPosBuffer, 0);

    // Clip-space coordinates (z = depth, w = 1.0)
    vec4 position_ccs = vec4(uv * 2.0 - 1.0, depth, 1.0);

    // Reconstruct view-space position by multiplying with the inverse projection matrix
    vec4 position_vcs = camera.invProj * position_ccs;

    // Perform perspective division to get the final view-space position
    position_vcs /= position_vcs.w;

    return position_vcs.xyz;
}

bool isReflective(vec3 R, float threshold) {
    return any(greaterThan(R, vec3(threshold)));
}

vec3 performSSR(
    SSR settings,
    vec3 viewPos,
    vec3 viewNormal,
    vec3 modelPos,
    sampler2D positionBuffer,
    sampler2D shadedBuffer,
    float metalness,
    float roughness,
    vec3 fresnel
) {
    // Reflection vector
    vec3 refl = normalize(reflect(normalize(viewPos), viewNormal));

                //False Importance Sampling of BRDF Roughness

    vec3 jitt = mix(vec3(0.0), vec3(hash0(modelPos, vec3(.8, .8, .8), 19.19)), roughness);

                //Raymarch through depth buffer
    vec3 hitPos = viewPos;
    vec2 hitCoord = vec2(0.0);
    bool hit = raymarchVCS(settings, positionBuffer, viewPos, refl, hitCoord, hitPos);
    hitCoord = hit ? hitCoord / textureSize(positionBuffer, 0) : vec2(-1.0);
                // hitCoord = clamp(hitCoord,vec2(0,0),vec2(1.0));
    vec3 reflectionColour = hit ? texture(shadedBuffer, hitCoord).rgb : vec3(0.0);

                //Control edges
    vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - hitCoord.xy));
    float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
    vec3 reflectionPower = pow(metalness, REFLECTION_FALLOFF_EXP) * screenEdgefactor * -refl.z * fresnel;
                // vec3 reflectionPower    = fresnel * -refl.z* screenEdgefactor;

                // mat3 rotY = rotationY(radians(scene.envRotation));
                // reflectionColour += !hit && scene.useIBL ?  texture(irradianceMap, normalize(rotY*(camera.invView*vec4(refl,0.0)).xyz)).rgb*scene.ambientIntensity : vec3(0.0);

    return clamp(reflectionPower, 0.0, 0.9) * reflectionColour;
                // direct =  reflectionColour;
                // direct = vec3(hitCoord,hit);
}