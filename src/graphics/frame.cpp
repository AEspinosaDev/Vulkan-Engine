#include <engine/graphics/frame.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

bool Frame::guiEnabled = false;

void Frame::cleanup() {
    commandPool.cleanup();
    renderFence.cleanup();
    renderSemaphore.cleanup();
    presentSemaphore.cleanup();
}

} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END