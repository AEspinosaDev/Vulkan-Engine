/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef IMAGE_H
#define IMAGE_H

#include <engine/graphics/buffer.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/utilities/translator.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct ImageConfig {
    ColorFormatType format     = ColorFormatType::SRGBA_8;
    ImageUsageFlags usageFlags = IMAGE_USAGE_SAMPLED;
    uint16_t        samples    = 1U;
    uint32_t        mipLevels  = 1U;
    uint32_t        layers     = 1U;
    ImageLayout     layout     = LAYOUT_UNDEFINED;
    ClearValue      clearValue = {{{0.0, 0.0, 0.0, 1.0}}};
    /*View*/
    ImageAspect aspectFlags  = ASPECT_COLOR;
    TextureType viewType     = TEXTURE_2D;
    uint32_t    baseMipLevel = 0;
};

struct SamplerConfig {
    FilterType  filters            = FILTER_LINEAR;
    MipmapMode  mipmapMode         = MIPMAP_LINEAR;
    AddressMode samplerAddressMode = ADDRESS_MODE_REPEAT;
    float       minLod             = 0.0f;
    float       maxLod             = 12.0f;
    bool        anysotropicFilter  = false;
    float       maxAnysotropy      = 1.0f;
    BorderColor border             = BorderColor::FLOAT_OPAQUE_WHITE;
};

struct Image {

    VkImage         handle        = VK_NULL_HANDLE;
    VkDevice        device        = VK_NULL_HANDLE;
    VmaAllocator    memory        = VK_NULL_HANDLE; /*Memory allocation managed by VMA*/
    VmaAllocation   allocation    = VK_NULL_HANDLE;
    VkImageView     view          = VK_NULL_HANDLE;
    VkSampler       sampler       = VK_NULL_HANDLE;
    VkDescriptorSet GUIReadHandle = VK_NULL_HANDLE;

    /*Config parameters*/
    Extent3D      extent        = {0, 0, 1}; // Depth for 3D Textures
    ImageLayout   currentLayout = LAYOUT_UNDEFINED;
    ImageConfig   config;
    SamplerConfig samplerConfig;

    bool loadedOnCPU{false};
    bool loadedOnGPU{false};

    void create_view(ImageConfig _config);

    void create_sampler(SamplerConfig _samplerConfig);

    void create_GUI_handle();

    void cleanup(bool destroySampler = true);

    Image clone() const;

    // ~Image() {
    //     cleanup(); 
    // }
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif