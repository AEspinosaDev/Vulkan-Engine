#include <engine/graphics/accel.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void BLAS::cleanup() {
    if (handle)
    {
        vkDestroyAccelerationStructure(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
        buffer.cleanup();
    }
}
void TLAS::cleanup() {

    if (handle)
    {
        vkDestroyAccelerationStructure(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
        buffer.cleanup();
    }
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END