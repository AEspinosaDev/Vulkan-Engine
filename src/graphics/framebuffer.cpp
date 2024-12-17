#include <engine/graphics/framebuffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {

void Framebuffer::cleanup() {
    if (handle)
    {
        vkDestroyFramebuffer(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
    for (size_t i = 0; i < attachmentImages.size(); i++)
    {
        attachmentImages[i].cleanup();
    }
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END