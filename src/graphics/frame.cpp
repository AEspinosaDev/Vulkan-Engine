#include <engine/graphics/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

bool Frame::guiEnabled = false;

void Frame::init(VkDevice &device, VkPhysicalDevice &gpu, VkSurfaceKHR surface)
{
    // create a command pool for commands submitted to the graphics queue.
    // we also want the pool to allow for resetting of individual command buffers
    VkCommandPoolCreateInfo commandPoolInfo = Init::command_pool_create_info(
        Utils::find_queue_families(gpu, surface).graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));

    // allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo cmdAllocInfo = Init::command_buffer_allocate_info(commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &commandBuffer));

    // create syncronization structures
    // one fence to control when the gpu has finished rendering the frame,
    // and 2 semaphores to syncronize rendering with swapchain
    // we want the fence to start signalled so we can wait on it on the first frame
    VkSemaphoreCreateInfo semaphoreCreateInfo = Init::semaphore_create_info();
    VkFenceCreateInfo fenceCreateInfo = Init::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore));
}

void Frame::cleanup(VkDevice &device)
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyFence(device, renderFence, nullptr);
    vkDestroySemaphore(device, presentSemaphore, nullptr);
    vkDestroySemaphore(device, renderSemaphore, nullptr);
}

} // namespace render
VULKAN_ENGINE_NAMESPACE_END