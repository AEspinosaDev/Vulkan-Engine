#include <engine/graphics/semaphore.h>


VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {

void Semaphore::init(VkDevice device) {
    m_device                                  = device;
    VkSemaphoreCreateInfo semaphoreCreateInfo = Init::semaphore_create_info();
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_handle));
}
void Semaphore::cleanup() {
    if (m_handle != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(m_device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}
void Fence::init(VkDevice device) {
    m_device                          = device;
    VkFenceCreateInfo fenceCreateInfo = Init::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &m_handle));
}
void Fence::cleanup() {
    if (m_handle != VK_NULL_HANDLE)
    {
        vkDestroyFence(m_device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}
void Fence::reset() {
    if (m_handle != VK_NULL_HANDLE)
        VK_CHECK(vkResetFences(m_device, 1, &m_handle));
}
void Fence::wait(uint64_t timeout) {
    VK_CHECK(vkWaitForFences(m_device, 1, &m_handle, VK_TRUE, timeout));
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END