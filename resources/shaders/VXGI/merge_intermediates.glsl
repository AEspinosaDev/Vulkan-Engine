#shader compute
#version 460 core

// In this case the voxelizer atomically writes to 3 seperate r32ui textures
// instead of only one rgba16f texture. Here we merge them together into one.

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

layout(set = 0,  binding =  6, r32f)    uniform image3D         finalVoxelImage;
layout(set = 0,  binding =  8)          uniform usampler3D      interVoxelImages[3];

void main()
{
    ivec3 imgCoord = ivec3(gl_GlobalInvocationID);

    float a = imageLoad(finalVoxelImage, imgCoord).a;
    if (a > 0.0)
    {
        uint r = texelFetch(interVoxelImages[0], imgCoord, 0).r;
        uint g = texelFetch(interVoxelImages[1], imgCoord, 0).r;
        uint b = texelFetch(interVoxelImages[2], imgCoord, 0).r;
        imageStore(finalVoxelImage, imgCoord, vec4(uintBitsToFloat(r), uintBitsToFloat(g), uintBitsToFloat(b), a));
    }
}