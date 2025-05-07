#include <engine/graphics/accel.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Accel::cleanup() {
    if (handle)
    {
        vkDestroyAccelerationStructure(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
        buffer.cleanup();
        instances = 0;
    }
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END