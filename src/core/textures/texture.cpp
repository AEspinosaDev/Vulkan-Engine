#include <engine/core/textures/texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core
{
Texture *Texture::DEBUG_TEXTURE = nullptr;

Graphics::Image *const get_image(Texture *t)
{
    TextureSettings textSettings = t->m_settings;
    t->m_image.config.format = static_cast<VkFormat>(textSettings.format);
    t->m_image.samplerConfig.anysotropicFilter = textSettings.anisotropicFilter;
    t->m_image.samplerConfig.filters = static_cast<VkFilter>(textSettings.filter);
    t->m_image.samplerConfig.samplerAddressMode = static_cast<VkSamplerAddressMode>(textSettings.adressMode);

    return &t->m_image;
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END