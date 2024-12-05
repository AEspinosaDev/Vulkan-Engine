#shader compute
#version 450

layout(local_size_x = 32, local_size_y = 32) in;

layout(set = 0, binding = 1, r32f) uniform image2D mips[6];

layout(push_constant) uniform Mipmap {
    int srcLevel; 
    int dstLevel; 
} mip;


void main() {
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 srcCoord = dstCoord / 2;
    vec3 texelColor = vec3(0.0);


   // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    ivec2 offsetX = ivec2(1, 0); // Horizontal offset (1 texel in x)
    ivec2 offsetY = ivec2(0, 1); // Vertical offset (1 texel in y)

    vec3 a = imageLoad(mips[mip.srcLevel], srcCoord - offsetX + offsetY).rgb;
    vec3 b = imageLoad(mips[mip.srcLevel], srcCoord + offsetY).rgb;
    vec3 c = imageLoad(mips[mip.srcLevel], srcCoord + offsetX + offsetY).rgb;

    vec3 d = imageLoad(mips[mip.srcLevel], srcCoord - offsetX).rgb;
    vec3 e = imageLoad(mips[mip.srcLevel], srcCoord).rgb; // Center texel
    vec3 f = imageLoad(mips[mip.srcLevel], srcCoord + offsetX).rgb;

    vec3 g = imageLoad(mips[mip.srcLevel], srcCoord - offsetX - offsetY).rgb;
    vec3 h = imageLoad(mips[mip.srcLevel], srcCoord - offsetY).rgb;
    vec3 i = imageLoad(mips[mip.srcLevel], srcCoord + offsetX - offsetY).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    texelColor = e*4.0;
    texelColor += (b+d+f+h)*2.0;
    texelColor += (a+c+g+i);
    texelColor *= 1.0 / 16.0;
    
    imageStore(mips[mip.dstLevel], dstCoord ,vec4(texelColor,1.0));
}
