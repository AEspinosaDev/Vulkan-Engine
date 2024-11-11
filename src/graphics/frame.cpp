#include <engine/graphics/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

bool Frame::guiEnabled = false;

void Frame::init(VkDevice _device, VkPhysicalDevice gpu, VkSurfaceKHR surface) {
    device = _device;

    commandPool = new CommandPool();
    commandPool->init(device,
                     Utils::find_queue_families(gpu, surface).graphicsFamily.value(),
                     VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandBuffer = new CommandBuffer();
    commandBuffer->init(device, *commandPool);

    // create syncronization structures
    // one fence to control when the gpu has finished rendering the frame,
    // and 2 semaphores to syncronize rendering with swapchain
    // we want the fence to start signalled so we can wait on it on the first frame
    VkSemaphoreCreateInfo semaphoreCreateInfo = Init::semaphore_create_info();
    VkFenceCreateInfo     fenceCreateInfo     = Init::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore));
}

void Frame::cleanup() {
    commandPool->cleanup();
    vkDestroyFence(device, renderFence, nullptr);
    vkDestroySemaphore(device, presentSemaphore, nullptr);
    vkDestroySemaphore(device, renderSemaphore, nullptr);
}

} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END