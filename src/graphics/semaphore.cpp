#include <engine/graphics/semaphore.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {

void Semaphore::cleanup() {
    if (handle)
    {
        vkDestroySemaphore(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}
void Fence::cleanup() {
    if (handle)
    {
        vkDestroyFence(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}
void Fence::reset() {
    if (handle)
        VK_CHECK(vkResetFences(device, 1, &handle));
}
void Fence::wait(uint64_t timeout) {
    if (handle)
        VK_CHECK(vkWaitForFences(device, 1, &handle, VK_TRUE, timeout));
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END