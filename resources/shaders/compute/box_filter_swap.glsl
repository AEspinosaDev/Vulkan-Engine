#shader compute
#version 460

layout(set = 0, binding = 7) uniform sampler2D  InputImage; 
layout(set = 0, binding = 8) uniform image2D    OutputImage; 


layout(local_size_x = 16, local_size_y = 16) in;

void main() {
    // Get the image coordinates of the current thread
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    // // Read the R channel from the input image
    // float rValue = texture(uInputImage, pixelCoord).r;

    // // Write the value into the G channel of the output image
    // vec4 color = texture(uOutputImage, pixelCoord); // Load existing data in output image
    // color.g = rValue; // Store R value into G channel

    // Store the updated color back into the output image
    imageStore(OutputImage, pixelCoord, vec4(1.0));
}
