#ifndef VK_IMAGE
#define VK_IMAGE

#include "vk_buffer.h"
#include "vk_initializers.h"

VULKAN_ENGINE_NAMESPACE_BEGIN
struct Image
{
    VkImage image;
    VkImageView view;

    VkExtent3D extent;
    VkFormat format;

    uint32_t mipLevels{1};

    VmaAllocation allocation;

    void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples);
    void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VmaAllocationCreateInfo &allocInfo, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples);

    void create_view(VkDevice &device, VkImageAspectFlags aspectFlags);

    void upload_image(VkCommandBuffer &cmd, Buffer *stagingBuffer);

    void generate_mipmaps(VkCommandBuffer &cmd);

    void cleanup(VkDevice &device, VmaAllocator &memory);

    static const int BYTES_PER_PIXEL{4};

    // static void create_sampler(VkDevice &device,);
};

VULKAN_ENGINE_NAMESPACE_END

#endif