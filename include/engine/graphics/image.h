/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef IMAGE_H
#define IMAGE_H

#include <engine/graphics/buffer.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct ImageConfig {
    VkFormat              format     = VK_FORMAT_B8G8R8A8_SRGB;
    VkImageUsageFlags     usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkSampleCountFlagBits samples    = VK_SAMPLE_COUNT_1_BIT;
    uint32_t              mipLevels  = 1U;
    uint32_t              layers     = 1U;
};

struct ViewConfig {
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageViewType    viewType    = VK_IMAGE_VIEW_TYPE_2D;
};

struct SamplerConfig {
    VkFilter             filters            = VK_FILTER_LINEAR;
    VkSamplerMipmapMode  mipmapMode         = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    float                minLod             = 0.0f;
    float                maxLod             = 1.0f;
    bool                 anysotropicFilter  = false;
    float                maxAnysotropy      = 1.0f;
    VkBorderColor        border             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
};

struct Image {

    VkImage    handle;
    VkExtent3D extent;
    VkDevice   device;
    /*Memory allocation controlled by VMA*/
    VmaAllocator  memory;
    VmaAllocation allocation;
    ImageConfig   config{};
    VkImageView   view;
    ViewConfig    viewConfig{};
    VkSampler     sampler;
    SamplerConfig samplerConfig{};

    bool loadedOnCPU{false};
    bool loadedOnGPU{false};

    bool isInitialized{false};
    bool hasView{false};
    bool hasSampler{false};

    void init(VkDevice&      device,
              VmaAllocator   memory,
              bool           useMipmaps,
              VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);

    void create_view();

    void create_sampler();

    void upload_image(VkCommandBuffer& cmd, Buffer* stagingBuffer);

    void generate_mipmaps(VkCommandBuffer& cmd);

    void cleanup(bool destroySampler = true);
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif