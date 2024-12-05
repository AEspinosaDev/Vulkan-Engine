#shader compute
#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(set = 0, binding = 0) uniform sampler2D     srcImage; 
layout(set = 0, binding = 1, r32f) uniform image2D mips[6];

layout(push_constant) uniform Mipmap {
    int srcLevel; 
    int dstLevel; 
} mip;

// #define SAMPLE_TEXEL(img,coord) ()

// vec3 bilinearSample(ivec2 texCoord, vec2 offset) {
//     // Fractional part of the offset
//     vec2 frac = fract(offset);

//     // Four neighboring texels
//     vec3 c00;
//     vec3 c10;
//     vec3 c01;
//     vec3 c11;
//     if(mip.srcLevel>0){
//         c00 = imageLoad(srcImage, texCoord).rgb;
//         c10 = imageLoad(srcImage, texCoord + ivec2(1, 0)).rgb;
//         c01 = imageLoad(srcImage, texCoord + ivec2(0, 1)).rgb;
//         c11 = imageLoad(srcImage, texCoord + ivec2(1, 1)).rgb;
//     else{
//         c00 = texelFetch(srcImage, texCoord).rgb;
//         c10 = texelFetch(srcImage, texCoord + ivec2(1, 0)).rgb;
//         c01 = texelFetch(srcImage, texCoord + ivec2(0, 1)).rgb;
//         c11 = texelFetch(srcImage, texCoord + ivec2(1, 1)).rgb;
//     }
//     // Bilinear interpolation
//     vec3 interpX0 = mix(c00, c10, frac.x); // Horizontal interpolation at row 0
//     vec3 interpX1 = mix(c01, c11, frac.x); // Horizontal interpolation at row 1
//     vec3 result = mix(interpX0, interpX1, frac.y); // Vertical interpolation

//     return result;
// }


void main() {
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 srcCoord = dstCoord*2;
    vec3 texelColor = vec3(0.0);

    ivec2 offset2 = ivec2(2, 2);
    ivec2 offset1 = ivec2(1, 1);
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    if(mip.srcLevel==0){

       // Sample 13 neighboring texels around the current texel
        vec3 a = texelFetch(srcImage, srcCoord - ivec2(offset2.x, -offset2.y), 0).rgb;
        vec3 b = texelFetch(srcImage, srcCoord - ivec2(0, -offset2.y), 0).rgb;
        vec3 c = texelFetch(srcImage, srcCoord - ivec2(-offset2.x, -offset2.y), 0).rgb;

        vec3 d = texelFetch(srcImage, srcCoord - ivec2(offset2.x, 0), 0).rgb;
        vec3 e = texelFetch(srcImage, srcCoord, 0).rgb; // Current texel
        vec3 f = texelFetch(srcImage, srcCoord - ivec2(-offset2.x, 0), 0).rgb;

        vec3 g = texelFetch(srcImage, srcCoord - ivec2(offset2.x, offset2.y), 0).rgb;
        vec3 h = texelFetch(srcImage, srcCoord - ivec2(0, offset2.y), 0).rgb;
        vec3 i = texelFetch(srcImage, srcCoord - ivec2(-offset2.x, offset2.y), 0).rgb;

        vec3 j = texelFetch(srcImage, srcCoord - ivec2(offset1.x, -offset1.y), 0).rgb;
        vec3 k = texelFetch(srcImage, srcCoord - ivec2(-offset1.x, -offset1.y), 0).rgb;
        vec3 l = texelFetch(srcImage, srcCoord - ivec2(offset1.x, offset1.y), 0).rgb;
        vec3 m = texelFetch(srcImage, srcCoord - ivec2(-offset1.x, offset1.y), 0).rgb;

        // texelColor = texelFetch(srcImage, ivec2(dstCoord * 2), 0).rgb;
        texelColor = e*0.125;                // ok
	    texelColor += (a+c+g+i)*0.03125;     // ok
	    texelColor += (b+d+f+h)*0.0625;      // ok
	    texelColor += (j+k+l+m)*0.125;       // ok



    }else{


        // Sample 13 neighboring texels around the current texel
        vec3 a = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(offset2.x, -offset2.y)).rgb;
        vec3 b = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(0, -offset2.y)).rgb;
        vec3 c = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(-offset2.x, -offset2.y)).rgb;

        vec3 d = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(offset2.x, 0)).rgb;
        vec3 e = imageLoad(mips[mip.srcLevel], srcCoord).rgb; // Current texel
        vec3 f = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(-offset2.x, 0)).rgb;

        vec3 g = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(offset2.x, offset2.y)).rgb;
        vec3 h = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(0, offset2.y)).rgb;
        vec3 i = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(-offset2.x, offset2.y)).rgb;

        vec3 j = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(offset1.x, -offset1.y)).rgb;
        vec3 k = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(-offset1.x, -offset1.y)).rgb;
        vec3 l = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(offset1.x, offset1.y)).rgb;
        vec3 m = imageLoad(mips[mip.srcLevel], srcCoord - ivec2(-offset1.x, offset1.y)).rgb;

        texelColor = e*0.125;                // ok
	    texelColor += (a+c+g+i)*0.03125;     // ok
	    texelColor += (b+d+f+h)*0.0625;      // ok
	    texelColor += (j+k+l+m)*0.125;       // ok
        // texelColor = imageLoad(mips[mip.srcLevel], srcCoord).rgb;

    }
    imageStore(mips[mip.dstLevel], dstCoord ,vec4(texelColor,1.0));
}
