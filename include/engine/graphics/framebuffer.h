/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <engine/common.h>
#include <engine/graphics/renderpass.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
/*Vulkan FBO. Device should populate the struct*/
struct Framebuffer {
    VkFramebuffer handle = VK_NULL_HANDLE;
    VkDevice      device = VK_NULL_HANDLE;

    Extent2D extent = {0, 0};
    uint32_t layers = 1;

    std::vector<Image*> attachmentImagesPtrs;

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif