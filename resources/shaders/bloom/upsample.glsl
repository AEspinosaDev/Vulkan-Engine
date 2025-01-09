#shader compute
#version 450
// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

layout(local_size_x = 16, local_size_y = 16) in;

// Both bindings point to the same image view.
#define MIPMAP_LEVELS 6
layout(set = 0, binding = 2, r32f) uniform image2D  mipImages   [MIPMAP_LEVELS];
layout(set = 0, binding = 3) uniform sampler2D      mipSamplers [MIPMAP_LEVELS];

layout(push_constant) uniform Mipmap {
    int srcLevel; 
    int dstLevel; 
} mip;

void main() {
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dstSize =  imageSize(mipImages[mip.dstLevel]);

    if (dstCoord.x >= dstSize.x || dstCoord.y >= dstSize.y) {
        return;
    }

    vec3 texelColor = vec3(0.0);
    vec2 texCoord = (vec2(dstCoord)+0.5)/vec2(dstSize);

    float x = 0.00005;
    float y = 0.00005;

   
    vec3 a = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 b = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x,     texCoord.y + y)).rgb;
    vec3 c = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + x, texCoord.y + y)).rgb;

    vec3 d = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - x, texCoord.y)).rgb;
    vec3 e = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x,     texCoord.y)).rgb;
    vec3 f = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + x, texCoord.y)).rgb;

    vec3 g = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 h = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x,     texCoord.y - y)).rgb;
    vec3 i = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + x, texCoord.y - y)).rgb;
   

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    texelColor = e*4.0;
    texelColor += (b+d+f+h)*2.0;
    texelColor += (a+c+g+i);
    texelColor *= (1.0 / 16.0);
    texelColor += imageLoad(mipImages[mip.dstLevel],dstCoord).rgb; //Blend
    
    imageStore(mipImages[mip.dstLevel], dstCoord ,vec4(texelColor,1.0));


    
}
