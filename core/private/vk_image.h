#ifndef VK_IMAGE
#define VK_IMAGE

#include <stb_image.h>
#include "vk_buffer.h"


namespace vke
{
    struct Image
    {
        VkImage image;
        VmaAllocation allocation;
        VkFormat format;

		
        bool load_image(const char* file);
    };

} // namespace vke

#endif