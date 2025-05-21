#include <engine/graphics/framebuffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {

void Framebuffer::cleanup() {

    if (handle)
    {
        vkDestroyFramebuffer(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    for (size_t i = 0; i < attachmentViews.size(); i++)
    {
        vkDestroyImageView(device, attachmentViews[i], VK_NULL_HANDLE);
    }
    attachmentViews.clear();
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END