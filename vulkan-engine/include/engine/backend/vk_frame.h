/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VK_FRAME
#define VK_FRAME

#include "vk_buffer.h"
#include "vk_bootstrap.h"
#include "vk_initializers.h"
#include "vk_descriptors.h"

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

    DescriptorSet objectDescriptor;
    Buffer objectUniformBuffer;

    void init(VkDevice &device, VkPhysicalDevice &gpu, VkSurfaceKHR &surface);
    void cleanup(VkDevice &device);
};

VULKAN_ENGINE_NAMESPACE_END
#endif