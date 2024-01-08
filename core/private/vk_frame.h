#ifndef VK_FRAME
#define VK_FRAME

#include "vk_buffer.h"
#include "vk_bootstrap.h"
#include "vk_initializers.h"
#include "vk_descriptors.h"

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

        DescriptorSet objectDescriptor;
        Buffer objectUniformBuffer;

        void init(VkDevice &device, VkPhysicalDevice &gpu, VkSurfaceKHR &surface);
        void cleanup(VkDevice &device);
    };

}
#endif