#ifndef VK_IMAGE
#define VK_IMAGE

#include "vk_buffer.h"
#include "vk_initializers.h"

namespace vke
{
    struct Image
    {
        VkImage image;
        VkExtent3D extent;
        VkFormat format;

        VmaAllocation allocation;


        static const int BYTES_PER_PIXEL{4};

        void init(VmaAllocator memory, Buffer* stagingBuffer, unsigned char *pixels, int width, int height, int depth=1);

        void upload_image(VkCommandBuffer cmd, Buffer* stagingBuffer);

        void cleanup(VmaAllocator memory);
    };

} // namespace vke

#endif