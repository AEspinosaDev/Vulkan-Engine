#include <engine/graphics/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

bool Frame::guiEnabled = false;

void Frame::cleanup() {
    for (Buffer& buffer : uniformBuffers)
    {
        buffer.cleanup();
    }
    commandPool.cleanup();
    computeCommandPool.cleanup();
    renderFence.cleanup();
    renderSemaphore.cleanup();
    presentSemaphore.cleanup();
}

} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END