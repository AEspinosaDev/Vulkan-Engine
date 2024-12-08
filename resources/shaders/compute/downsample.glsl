#shader compute
#version 450
// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0) uniform sampler2D     srcImage; 
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

    vec2 srcTexelSize = 1.0 / imageSize(mipImages[mip.srcLevel]);
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;
    vec2 texCoord =  (vec2(dstCoord)+0.5)/vec2(dstSize); //Remember that the center of the pixel in UV is the value between numbers

    vec3 texelColor = vec3(0.0);

   
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    if(mip.srcLevel==0){

       // Sample 13 neighboring texels around the current texel
        vec3 a = texture(srcImage, vec2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
        vec3 b = texture(srcImage, vec2(texCoord.x,       texCoord.y + 2*y)).rgb;
        vec3 c = texture(srcImage, vec2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;

        vec3 d = texture(srcImage, vec2(texCoord.x - 2*x, texCoord.y)).rgb;
        vec3 e = texture(srcImage, vec2(texCoord.x,       texCoord.y)).rgb;
        vec3 f = texture(srcImage, vec2(texCoord.x + 2*x, texCoord.y)).rgb;

        vec3 g = texture(srcImage, vec2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
        vec3 h = texture(srcImage, vec2(texCoord.x,       texCoord.y - 2*y)).rgb;
        vec3 i = texture(srcImage, vec2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;

        vec3 j = texture(srcImage, vec2(texCoord.x - x, texCoord.y + y)).rgb;
        vec3 k = texture(srcImage, vec2(texCoord.x + x, texCoord.y + y)).rgb;
        vec3 l = texture(srcImage, vec2(texCoord.x - x, texCoord.y - y)).rgb;
        vec3 m = texture(srcImage, vec2(texCoord.x + x, texCoord.y - y)).rgb;

        texelColor = e*0.125;                // ok
	    texelColor += (a+c+g+i)*0.03125;     // ok
	    texelColor += (b+d+f+h)*0.0625;      // ok
	    texelColor += (j+k+l+m)*0.125;       // ok
    }else{
       // Sample 13 neighboring texels around the current texel
        vec3 a = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
        vec3 b = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x,       texCoord.y + 2*y)).rgb;
        vec3 c = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;

        vec3 d = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - 2*x, texCoord.y)).rgb;
        vec3 e = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x,       texCoord.y)).rgb;
        vec3 f = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + 2*x, texCoord.y)).rgb;

        vec3 g = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
        vec3 h = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x,       texCoord.y - 2*y)).rgb;
        vec3 i = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;

        vec3 j = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - x, texCoord.y + y)).rgb;
        vec3 k = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + x, texCoord.y + y)).rgb;
        vec3 l = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x - x, texCoord.y - y)).rgb;
        vec3 m = texture(mipSamplers[mip.srcLevel], vec2(texCoord.x + x, texCoord.y - y)).rgb;

        texelColor = e*0.125;                // ok
	    texelColor += (a+c+g+i)*0.03125;     // ok
	    texelColor += (b+d+f+h)*0.0625;      // ok
	    texelColor += (j+k+l+m)*0.125;       // ok

    }
    imageStore(mipImages[mip.dstLevel], dstCoord ,vec4(texelColor,1.0));
}
