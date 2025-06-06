/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_H
#define TEXTURE_H

#include <engine/graphics/image.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/utilities/translator.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct TextureConfig {

    TextureType type         = TEXTURE_2D;
    uint32_t    maxMipLevel  = 1U;
    uint32_t    baseMipLevel = 0;
    uint32_t    maxLayer     = 1U;
    uint32_t    minLayer     = 0;

    bool        sampled            = true;
    FilterType  filters            = FILTER_LINEAR;
    MipmapMode  mipmapMode         = MIPMAP_LINEAR;
    AddressMode samplerAddressMode = ADDRESS_MODE_REPEAT;
    float       maxAnysotropy      = 1.0f;
    BorderColor border             = BorderColor::FLOAT_OPAQUE_WHITE;

    ImageLayout expectedLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    bool operator==( const TextureConfig& other ) const {
        return type == other.type &&
               maxMipLevel == other.maxMipLevel &&
               baseMipLevel == other.baseMipLevel &&
               maxLayer == other.maxLayer &&
               minLayer == other.minLayer &&
               sampled == other.sampled &&
               filters == other.filters &&
               mipmapMode == other.mipmapMode &&
               samplerAddressMode == other.samplerAddressMode &&
               maxAnysotropy == other.maxAnysotropy &&
               border == other.border &&
               expectedLayout == other.expectedLayout;
    }
};

struct Texture {

    VkImageView     viewHandle    = VK_NULL_HANDLE;
    VkSampler       samplerHandle = VK_NULL_HANDLE;
    VkDescriptorSet GUIReadHandle = VK_NULL_HANDLE;

    /* Points to an existing image */
    Image* image { nullptr };

    /* Config */
    TextureConfig config {};

    const Extent3D& get_extent() const {
        return image->extent;
    }
    ColorFormatType get_format() const {
        return image->config.format;
    }

    void create_GUI_handle();

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif