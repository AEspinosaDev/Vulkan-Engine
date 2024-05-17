/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAME_H
#define FRAME_H

#include <engine/backend/buffer.h>
#include <engine/backend/bootstrap.h>
#include <engine/backend/initializers.h>
#include <engine/backend/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct Frame
{
    // Control
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
    VkFence renderFence;

    // Command
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    // GlobalLayout Descriptor
    DescriptorSet globalDescriptor;
    Buffer globalUniformBuffer;

    // ObjectLayout descriptors
    DescriptorSet objectDescriptor;
    Buffer objectUniformBuffer;

    void init(VkDevice &device, VkPhysicalDevice &gpu, VkSurfaceKHR surface);
    void cleanup(VkDevice &device);

    static bool guiEnabled;
};

VULKAN_ENGINE_NAMESPACE_END
#endif