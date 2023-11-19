#pragma once
#include "vk_core.h"

namespace vkeng
{
    struct Buffer
    {
        VkBuffer buffer;
        VmaAllocation allocation;
    };
}