#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {
void RenderPass::set_debug_name(const char* name) {

    debbugName                             = name;
    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType                         = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType                    = VK_OBJECT_TYPE_RENDER_PASS;
    nameInfo.objectHandle                  = reinterpret_cast<uint64_t>(handle);
    nameInfo.pObjectName                   = name;

    vkSetDebugUtilsObjectName(device, &nameInfo);
}

void RenderPass::cleanup() {

    if (handle)
    {
        vkDestroyRenderPass(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END