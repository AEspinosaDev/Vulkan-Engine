#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include "vk_bootstrap.h"

namespace vke
{
    struct Buffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;


        void upload_data(VmaAllocator memory, const void *bufferData, size_t size);
        void upload_data(VmaAllocator memory,const void *bufferData, size_t size, size_t offset);

    };
}

#endif