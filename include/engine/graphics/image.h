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
    ColorFormatType format        = ColorFormatType::SRGBA_8;
    ImageType       type          = IMAGE_TYPE_2D;
    ImageUsageFlags usageFlags    = IMAGE_USAGE_SAMPLED;
    uint16_t        samples       = 1U;
    uint32_t        mipmaps       = 1U;
    uint32_t        layers        = 1U;
    ClearValue      clearValue    = {{{0.0, 0.0, 0.0, 1.0}}};
    ImageLayout     initialLayout = LAYOUT_UNDEFINED;
};

struct Image {

    VkImage       handle     = VK_NULL_HANDLE;
    VkDevice      device     = VK_NULL_HANDLE;
    VmaAllocator  memory     = VK_NULL_HANDLE; /*Memory allocation managed by VMA*/
    VmaAllocation allocation = VK_NULL_HANDLE;

    /*Config parameters*/
    Extent3D    extent = {0, 0, 1};
    ImageConfig config = {};

    /*State*/
    ImageLayout currentLayout = LAYOUT_UNDEFINED;
    bool        empty         = true; // Is there data on the GPU ??

   
    void cleanup();
};


} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif