#shader compute
#version 450
// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

layout(local_size_x = 16, local_size_y = 16) in;

layout(set = 0, binding = 0) uniform sampler2D     srcImage; 
layout(set = 0, binding = 1, r32f) uniform image2D mips[6];

layout(push_constant) uniform Mipmap {
    int srcLevel; 
    int dstLevel; 
} mip;

#define EDGE_CLAMP(coords) (clamp(coords ,ivec2(0.0), mip.srcLevel > 0 ? ivec2(imageSize(mips[mip.srcLevel]).xy) : ivec2(textureSize(srcImage,0).xy)))

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

    if(dstCoord.x >  dstSize.x) return;
    if(dstCoord.y >  dstSize.y) return;

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
        vec2 srcTexelSize = 1.0 / imageSize(mips[mip.srcLevel]);
        float x = srcTexelSize.x;
        float y = srcTexelSize.y;
        vec2 texCoord = vec2(dstCoord)/vec2(dstSize);

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

        // texelColor = texelFetch(srcImage, ivec2(dstCoord * 2), 0).rgb;
        texelColor = e*0.125;                // ok
	    texelColor += (a+c+g+i)*0.03125;     // ok
	    texelColor += (b+d+f+h)*0.0625;      // ok
	    texelColor += (j+k+l+m)*0.125;       // ok
        // texelColor = texture(srcImage,srcCoord/imageSize(mips[mip.srcLevel])).rgb;



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
        //  texelColor = imageLoad(mips[mip.srcLevel], srcCoord).rgb;

    }
    imageStore(mips[mip.dstLevel], dstCoord ,vec4(texelColor,1.0));
}
