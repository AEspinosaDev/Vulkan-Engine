/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAME_H
#define FRAME_H

#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/descriptors.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

struct Frame
{
    // Control
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;

    // Command
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // Uniforms
    std::vector<Buffer> uniformBuffers;

    void init(VkDevice &device, VkPhysicalDevice &gpu, VkSurfaceKHR surface);

    void cleanup(VkDevice &device);

    static bool guiEnabled;
};

} // namespace render

VULKAN_ENGINE_NAMESPACE_END
#endif