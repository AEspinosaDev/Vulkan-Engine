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
    VkSampler sampler;

    VkExtent3D extent;
    VkFormat format;
    uint32_t layers;
    uint32_t mipLevels{1};

    bool isInitialized{false};
    bool hasView{false};
    bool hasSampler{false};

    VmaAllocation allocation;

    void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples, uint32_t imageLayers = 1);
    void init(VmaAllocator memory, VkFormat imageFormat, VkImageUsageFlags usageFlags, VmaAllocationCreateInfo &allocInfo, VkExtent3D imageExtent, bool useMipmaps, VkSampleCountFlagBits samples, uint32_t imageLayers = 1);

    void create_view(VkDevice &device, VkImageAspectFlags aspectFlags, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

    void create_sampler(VkDevice &device, VkFilter filters, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode samplerAddressMode, float minLod=0.0f, float maxLod = 1.0f, bool anysotropicFilter = false, float maxAnysotropy = 1.0f,VkBorderColor border = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

    void upload_image(VkCommandBuffer &cmd, Buffer *stagingBuffer);

    void generate_mipmaps(VkCommandBuffer &cmd);

    void cleanup(VkDevice &device, VmaAllocator &memory, bool destroySampler = true);

    static const int BYTES_PER_PIXEL{4};
};

VULKAN_ENGINE_NAMESPACE_END

#endif