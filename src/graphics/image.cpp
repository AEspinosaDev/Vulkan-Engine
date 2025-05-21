#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Image::cleanup() {
    if (handle && memory)
    {
        vmaDestroyImage(memory, handle, allocation);
        handle = VK_NULL_HANDLE;
    }
}



} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END