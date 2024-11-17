#include <engine/graphics/vk_renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {

void VulkanRenderPass::cleanup() {
    if (handle)
    {
        vkDestroyRenderPass(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END