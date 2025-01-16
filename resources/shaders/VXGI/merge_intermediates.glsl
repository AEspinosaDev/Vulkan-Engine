#shader compute
#version 460 core

// In this case the voxelizer atomically writes to 3 seperate r32ui textures
// instead of only one rgba16f texture. Here we merge them together into one.

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

layout(set = 0,  binding =  6, r32f)    uniform image3D finalVoxelImage;
layout(set = 0,  binding =  7, r32ui)   uniform uimage3D interVoxelImages[3];

void main()
{
    ivec3 imgCoord = ivec3(gl_GlobalInvocationID);

    float a = imageLoad(finalVoxelImage, imgCoord).a;
    if (a > 0.0)
    {
        float r = imageLoad(interVoxelImages[0], imgCoord).r;
        float g = imageLoad(interVoxelImages[1], imgCoord).r;
        float b = imageLoad(interVoxelImages[2], imgCoord).r;
        imageStore(finalVoxelImage, imgCoord, vec4(r, g, b, a));
    }
}