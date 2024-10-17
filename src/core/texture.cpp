#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

Texture *Texture::DEBUG_TEXTURE = nullptr;

graphics::Image *const get_image(Texture *t)
{
    TextureSettings textSettings = t->m_settings;
    t->m_image.config.format = static_cast<VkFormat>(textSettings.format);
    t->m_image.samplerConfig.anysotropicFilter = textSettings.anisotropicFilter;
    t->m_image.samplerConfig.filters = static_cast<VkFilter>(textSettings.filter);
    t->m_image.samplerConfig.samplerAddressMode = static_cast<VkSamplerAddressMode>(textSettings.adressMode);

    return &t->m_image;
}

VULKAN_ENGINE_NAMESPACE_END