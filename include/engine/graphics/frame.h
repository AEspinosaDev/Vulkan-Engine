/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAME_H
#define FRAME_H

#include <engine/graphics/command_buffer.h>
#include <engine/graphics/semaphore.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct Frame {
    // Control
    Semaphore presentSemaphore = {};
    Semaphore renderSemaphore  = {};
    Fence     renderFence      = {};
    // Command
    CommandPool   commandPool   = {};
    CommandBuffer commandBuffer = {};

    // GPU Global Buffers
    Buffer globalBuffer;
    Buffer objectBuffer;

    uint32_t index = 0;

    void cleanup();

    static bool guiEnabled;
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif