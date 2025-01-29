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

layout(location = 0) in  vec2 v_uv;

layout(set = 0, binding = 0) uniform sampler2D inputImage;
layout(push_constant) uniform Settings {
    float exposure;
    float  type; 
} settings;

layout(location = 0) out vec4 outputImage;

vec3 reindhartTonemap(vec3 color){
    color *= settings.exposure;
    return color / (color + vec3(1.0));
}

// Filmic Exponential Tonemapping
vec3 filmicTonemap(vec3 color){
    return vec3(1.0) - exp(-color * settings.exposure);
}

vec3 uncharted2Tonemap(vec3 color) {
    vec3 a = (color + vec3(0.004)) / (color + vec3(1.0));
    return a * (1.0 + a / (a + vec3(0.15)));
}

/*
 * ACES tonemapping fit for the sRGB color space
 * https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
 */
// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
    );

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
    );

vec3 RRT_ODT_Fit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESTonemapFitted(vec3 color)
{
    color = color * exp2(settings.exposure);
	color = ACESInputMat * color;
    color = RRT_ODT_Fit(color);
    color = ACESOutputMat * color;
    // return clamp(color, 0.0, 1.0);
    return color;
}

void main()
{
    vec3 result = texture(inputImage,v_uv).rgb;

    if(settings.type == 0.0)
        result = filmicTonemap(result);
    if(settings.type == 1.0)
        result = reindhartTonemap(result);
    if(settings.type == 2.0)
        result = ACESTonemapFitted(result);


    outputImage = vec4(result,1.0); 
}