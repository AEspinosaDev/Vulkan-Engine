#ifndef VK_FRAME_H
#define VK_FRAME_H

#include "vk_buffer.h"
#include "vk_bootstrap.h"
#include "vk_initializers.h"

namespace vke
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

        //Descriptors
        VkDescriptorSet globalDescriptor;
        
        VkDescriptorSet objectDescriptor;
        Buffer objectUniformBuffer;


        void init(VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface);
        void cleanup(VkDevice device);
       
    };

}
#endif