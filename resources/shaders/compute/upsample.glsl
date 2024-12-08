#shader compute
#version 450
// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 1, r32f) uniform image2D mips[6];

layout(push_constant) uniform Mipmap {
    int srcLevel; 
    int dstLevel; 
} mip;

#define EDGE_CLAMP(coords,size) (clamp(coords ,ivec2(0.0),size))

vec4 bilinearSample(int id, vec2 uv) {
    // Get the texture size (use imageSize() or textureSize() to get width/height)
    ivec2 texSize = imageSize(mips[id]);  // Level 0 of the mipmap chain
    
    // Get the floating point coordinates in the range [0, texSize-1]
    vec2 texCoord = uv * vec2(texSize);
    
    // Calculate the integer coordinates for the 4 texels surrounding the texture coordinate
    ivec2 p0 = ivec2(floor(texCoord.x), floor(texCoord.y));  // Bottom-left
    ivec2 p1 = p0 + ivec2(1, 0);                             // Bottom-right
    ivec2 p2 = p0 + ivec2(0, 1);                             // Top-left
    ivec2 p3 = p0 + ivec2(1, 1);                             // Top-right
    
    // Get the fractional part (used for interpolation weights)
    vec2 frac = texCoord - floor(texCoord);
    float x_frac = frac.x;
    float y_frac = frac.y;
    
    // Fetch the four surrounding texels
    vec4 color00 = imageLoad(mips[id], p0);  // Bottom-left
    vec4 color10 = imageLoad(mips[id], p1);  // Bottom-right
    vec4 color01 = imageLoad(mips[id], p2);  // Top-left
    vec4 color11 = imageLoad(mips[id], p3);  // Top-right
    
    // Interpolate between the texels along the x-axis (horizontally)
    vec4 interp_x0 = mix(color00, color10, x_frac);  // Bottom
    vec4 interp_x1 = mix(color01, color11, x_frac);  // Top
    
    // Interpolate between the results along the y-axis (vertically)
    vec4 result = mix(interp_x0, interp_x1, y_frac);  // Final bilinear interpolation
    
    return result;
}


void main() {
    ivec2 dstCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dstSize =  imageSize(mips[mip.dstLevel]);

     if (dstCoord.x >= dstSize.x || dstCoord.y >= dstSize.y) {
        return;
    }

    ivec2 srcCoord = dstCoord >> 1;
    vec3 texelColor = vec3(0.0);

    // vec2 uv = vec2(dstCoord)/vec2(dstSize);
    // vec2 offsetX = ivec2(0.005, 0); // Horizontal offset (1 texel in x)
    // vec2 offsetY = ivec2(0, 0.005); // Vertical offset (1 texel in y)

    // vec3 a = bilinearSample(mip.srcLevel,  (uv - offsetX + offsetY)).rgb;
    // vec3 b = bilinearSample(mip.srcLevel,  (uv + offsetY)).rgb;
    // vec3 c = bilinearSample(mip.srcLevel,  (uv + offsetX + offsetY)).rgb;

    // vec3 d = bilinearSample(mip.srcLevel,  (uv - offsetX)).rgb;
    // vec3 e = bilinearSample(mip.srcLevel, uv).rgb; // Center texel
    // vec3 f = bilinearSample(mip.srcLevel,  (uv + offsetX)).rgb;

    // vec3 g = bilinearSample(mip.srcLevel,  (uv - offsetX - offsetY)).rgb;
    // vec3 h = bilinearSample(mip.srcLevel,  (uv - offsetY)).rgb;
    // vec3 i = bilinearSample(mip.srcLevel,  (uv + offsetX - offsetY)).rgb;


//    Take 9 samples around current texel:
//     a - b - c
//     d - e - f
//     g - h - i
    ivec2 offsetX = ivec2(1, 0); // Horizontal offset (1 texel in x)
    ivec2 offsetY = ivec2(0, 1); // Vertical offset (1 texel in y)
    ivec2 pixelSize = imageSize(mips[mip.srcLevel]).xy;


    vec3 a = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord - offsetX + offsetY,pixelSize)).rgb;
    vec3 b = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord + offsetY,pixelSize)).rgb;
    vec3 c = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord + offsetX + offsetY,pixelSize)).rgb;

    vec3 d = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord - offsetX,pixelSize)).rgb;
    vec3 e = imageLoad(mips[mip.srcLevel], srcCoord).rgb; // Center texel
    vec3 f = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord + offsetX,pixelSize)).rgb;

    vec3 g = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord - offsetX - offsetY,pixelSize)).rgb;
    vec3 h = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord - offsetY,pixelSize)).rgb;
    vec3 i = imageLoad(mips[mip.srcLevel],  EDGE_CLAMP(srcCoord + offsetX - offsetY,pixelSize)).rgb;

    

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    texelColor = e*4.0;
    texelColor += (b+d+f+h)*2.0;
    texelColor += (a+c+g+i);
    texelColor *= (1.0 / 16.0);
    texelColor += imageLoad(mips[mip.dstLevel],dstCoord).rgb;
    
    imageStore(mips[mip.dstLevel], dstCoord ,vec4(texelColor,1.0));
    // imageStore(mips[mip.dstLevel], dstCoord ,vec4(imageLoad(mips[mip.srcLevel],srcCoord).rgb,1.0));


    
}
