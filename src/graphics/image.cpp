#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Image::create_view(ImageConfig config) {
    VkImageViewCreateInfo dview_info = Init::imageview_create_info(Translator::get(config.format),
                                                                   handle,
                                                                   Translator::get(config.viewType),
                                                                   Translator::get(config.aspectFlags),
                                                                   mipLevels,
                                                                   layers,
                                                                   baseMipLevel);
    VK_CHECK(vkCreateImageView(device, &dview_info, nullptr, &view));
}
void Image::create_sampler(SamplerConfig config) {
    VkSamplerCreateInfo samplerInfo =
        Init::sampler_create_info(Translator::get(config.filters),
                                  Translator::get(config.mipmapMode),
                                  config.minLod,
                                  config.maxLod <= mipLevels ? config.maxLod : mipLevels + baseMipLevel,
                                  config.anysotropicFilter,
                                  config.maxAnysotropy,
                                  Translator::get(config.samplerAddressMode));
    samplerInfo.borderColor = Translator::get(config.border);

    VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
}

void Image::create_GUI_handle() {
    if (GUIReadHandle == VK_NULL_HANDLE)
        GUIReadHandle =
            ImGui_ImplVulkan_AddTexture(sampler, view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
void Image::cleanup(bool destroySampler) {
    if (view)
    {
        vkDestroyImageView(device, view, nullptr);
        view = VK_NULL_HANDLE;
    }
    if (handle && memory)
    {
        vmaDestroyImage(memory, handle, allocation);
        handle = VK_NULL_HANDLE;
    }
    if (destroySampler && sampler)
    {
        vkDestroySampler(device, sampler, VK_NULL_HANDLE);
        sampler = VK_NULL_HANDLE;
    }
    if (GUIReadHandle)
    {
        ImGui_ImplVulkan_RemoveTexture(GUIReadHandle);
        GUIReadHandle = VK_NULL_HANDLE;
    }
}
Image Image::clone() const {
    Image img         = {};
    img.handle        = handle;
    img.device        = device;
    img.memory        = memory;
    img.allocation    = allocation;
    img.view          = view;
    img.sampler       = sampler;
    img.GUIReadHandle = GUIReadHandle;

    img.extent       = extent;
    img.layers       = layers;
    img.mipLevels    = mipLevels;
    img.baseMipLevel = baseMipLevel;

    return img;
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END