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

layout(location = 0) in vec2 v_uv;

layout(set = 0, binding = 0) uniform sampler2D inputImage;
layout(set = 0, binding = 1) uniform sampler2D velocityBuffer;
layout(set = 0, binding = 2) uniform sampler2D prevFilteredImage;

layout(location = 0) out vec4 outputImage;

void main() {
    vec2 velocity = texture(velocityBuffer, v_uv).xy;
    vec2 reprojectedUV = v_uv - velocity;

    vec3 historyColor = texture(prevFilteredImage, reprojectedUV).rgb;
    vec3 currentColor = texture(inputImage, v_uv).rgb;

    // Sample 8 neighbors
    ivec2 texSize = textureSize(inputImage, 0);
    vec3 NearColor0 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(1, 0), 0).xyz;
    vec3 NearColor1 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(0, 1), 0).xyz;
    vec3 NearColor2 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(-1, 0), 0).xyz;
    vec3 NearColor3 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(0, -1), 0).xyz;
    vec3 NearColor4 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(1, 1), 0).xyz;
    vec3 NearColor5 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(-1, 1), 0).xyz;
    vec3 NearColor6 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(1, -1), 0).xyz;
    vec3 NearColor7 = texelFetch(inputImage, ivec2(v_uv * texSize) + ivec2(-1, -1), 0).xyz;

    vec3 BoxMin = min(currentColor, min(min(min(NearColor0, NearColor1), min(NearColor2, NearColor3)), min(min(NearColor4, NearColor5), min(NearColor6, NearColor7))));
    vec3 BoxMax = max(currentColor, max(max(max(NearColor0, NearColor1), max(NearColor2, NearColor3)), max(max(NearColor4, NearColor5), max(NearColor6, NearColor7))));

    historyColor = clamp(historyColor, BoxMin, BoxMax);

    // Velocity magnitude
    float speed = length(velocity) * float(texSize.x);

// // Color difference between current and history
//     float colorDifference = length(currentColor - historyColor);

// // Dynamic alpha considering both
//     float kMinAlpha = 0.1;
//     float kMaxAlpha = 0.0;

//     float dynamicAlpha = mix(kMinAlpha, kMaxAlpha, clamp(max(speed * 50.0, colorDifference * 10.0), 0.0, 1.0));

    // Optional: kill history if crazy fast
    if(speed > 1.0)
        historyColor = currentColor;

    // Blend
    vec3 resolvedColor = mix(currentColor, historyColor, 0.2);

    outputImage = vec4(resolvedColor, 1.0);
}
