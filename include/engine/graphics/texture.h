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

    TextureType viewType     = TEXTURE_2D;
    ImageAspect aspectFlags  = ASPECT_COLOR;
    uint32_t    mipLevels    = 1U;
    uint32_t    baseMipLevel = 0;
    uint32_t    layers       = 1U;
    uint32_t    baseLayer    = 0;

    FilterType  filters            = FILTER_LINEAR;
    MipmapMode  mipmapMode         = MIPMAP_LINEAR;
    AddressMode samplerAddressMode = ADDRESS_MODE_REPEAT;
    bool        anysotropicFilter  = false;
    float       maxAnysotropy      = 1.0f;
    BorderColor border             = BorderColor::FLOAT_OPAQUE_WHITE;
};

struct Texture {

    VkImageView     viewHandle    = VK_NULL_HANDLE;
    VkSampler       samplerHandle = VK_NULL_HANDLE;
    VkDescriptorSet GUIReadHandle = VK_NULL_HANDLE;

    /* Points to an existing image */
    ptr<Image> image;

    /* Config */
    TextureConfig config{};

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif