#pragma once
#include "vk_buffer.h"
#include "vk_bootstrap.h"
#include "vk_initializers.h"

namespace vkeng
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

        // Descriptors
        VkDescriptorSet globalDescriptor;
        Buffer cameraUniformBuffer;

        // AllocatedBuffer objectBuffer;
        // VkDescriptorSet objectDescriptor;
        // DeletionQueue _frameDeletionQueue;

        void init(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface);
        void cleanup(VkDevice device);
       
    };

}