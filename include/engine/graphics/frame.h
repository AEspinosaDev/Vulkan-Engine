/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAME_H
#define FRAME_H

#include <engine/graphics/command_buffer.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct Frame {
    VkDevice device;
    // Control
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
    VkFence     renderFence;

    // Command
    CommandPool* commandPool;
    CommandBuffer* commandBuffer;

    // Uniforms
    std::vector<Buffer> uniformBuffers;

    void init(VkDevice _device, VkPhysicalDevice gpu, VkSurfaceKHR surface);

    void cleanup();

    static bool guiEnabled;
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif