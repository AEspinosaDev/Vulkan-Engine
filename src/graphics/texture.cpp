#include <engine/graphics/texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
void Texture::create_GUI_handle() {
    if (GUIReadHandle == VK_NULL_HANDLE)
        GUIReadHandle = ImGui_ImplVulkan_AddTexture(samplerHandle, viewHandle, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Texture::cleanup() {

    if (viewHandle)
    {
        vkDestroyImageView(image->device, viewHandle, VK_NULL_HANDLE);
        viewHandle = VK_NULL_HANDLE;
    }
    if (samplerHandle)
    {
        vkDestroySampler(image->device, samplerHandle, VK_NULL_HANDLE);
        samplerHandle = VK_NULL_HANDLE;
    }
    if (GUIReadHandle)
    {
        ImGui_ImplVulkan_RemoveTexture(GUIReadHandle);
        GUIReadHandle = VK_NULL_HANDLE;
    }
}

} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END