#include <engine/graphics/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

bool Frame::guiEnabled = false;

void Frame::init(VkDevice _device, VkPhysicalDevice gpu, VkSurfaceKHR surface, uint32_t id) {
    commandPool   = new CommandPool;
    commandBuffer = new CommandBuffer;

    commandPool->init(_device,
                      Utils::find_queue_families(gpu, surface).graphicsFamily.value(),
                      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    commandBuffer->init(_device, *commandPool);

    renderFence.init(_device);
    renderSemaphore.init(_device);
    presentSemaphore.init(_device);

    index = id;
}

void Frame::cleanup() {
    commandPool->cleanup();
    renderFence.cleanup();
    renderSemaphore.cleanup();
    presentSemaphore.cleanup();

    // delete commandPool;
    // delete commandBuffer;
}

} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END