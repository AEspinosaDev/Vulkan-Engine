#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include "vk_core.h"

namespace vke
{
    struct Buffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
    };
}

#endif