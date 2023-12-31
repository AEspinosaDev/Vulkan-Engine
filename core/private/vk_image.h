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

        VkImageView view;

        VmaAllocation allocation;

        static const int BYTES_PER_PIXEL{4};

        void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VkExtent3D imageExtent);
        void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VmaAllocationCreateInfo &allocInfo, VkExtent3D imageExtent);
        void create_view(VkDevice device, VkImageAspectFlags aspectFlags);

        void upload_image(VkCommandBuffer cmd, Buffer *stagingBuffer);

        void cleanup(VkDevice device, VmaAllocator memory);
    };

} // namespace vke

#endif