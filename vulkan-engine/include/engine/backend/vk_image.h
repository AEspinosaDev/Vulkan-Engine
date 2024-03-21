/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
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

    uint32_t layers;

    uint32_t mipLevels{1};

    VmaAllocation allocation;

    void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples, uint32_t imageLayers = 1);
    void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VmaAllocationCreateInfo &allocInfo, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples,uint32_t imageLayers = 1);

    void create_view(VkDevice &device, VkImageAspectFlags aspectFlags, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

    void upload_image(VkCommandBuffer &cmd, Buffer *stagingBuffer);

    void generate_mipmaps(VkCommandBuffer &cmd);

    void cleanup(VkDevice &device, VmaAllocator &memory);

    static const int BYTES_PER_PIXEL{4};

    // static void create_sampler(VkDevice &device,);
};

VULKAN_ENGINE_NAMESPACE_END

#endif