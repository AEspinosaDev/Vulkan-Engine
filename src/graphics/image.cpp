#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Image::create_view(ImageConfig _config) {
    VkImageViewCreateInfo dview_info = Init::imageview_create_info(Translator::get(_config.format),
                                                                   handle,
                                                                   Translator::get(_config.viewType),
                                                                   Translator::get(_config.aspectFlags),
                                                                   config.mipLevels,
                                                                   config.layers,
                                                                   config.baseMipLevel);
    VK_CHECK(vkCreateImageView(device, &dview_info, nullptr, &view));
}
void Image::create_sampler(SamplerConfig _samplerConfig) {
    VkSamplerCreateInfo samplerInfo =
        Init::sampler_create_info(Translator::get(_samplerConfig.filters),
                                  Translator::get(_samplerConfig.mipmapMode),
                                  _samplerConfig.minLod,
                                  _samplerConfig.maxLod <= config.mipLevels ? _samplerConfig.maxLod : config.mipLevels + config.baseMipLevel,
                                  _samplerConfig.anysotropicFilter,
                                  _samplerConfig.maxAnysotropy,
                                  Translator::get(_samplerConfig.samplerAddressMode));
    samplerInfo.borderColor = Translator::get(_samplerConfig.border);

    samplerConfig = _samplerConfig;

    VK_CHECK(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
}

void Image::create_GUI_handle() {
    if (GUIReadHandle == VK_NULL_HANDLE)
        GUIReadHandle = ImGui_ImplVulkan_AddTexture(sampler, view, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

    img.extent        = extent;
    img.config        = config;
    img.samplerConfig = samplerConfig;

    return img;
}


} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END