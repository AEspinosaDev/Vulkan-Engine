#include <engine/core/textures/texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core
{
Graphics::Image *const get_image(ITexture *t)
{
    return &t->m_image;
}

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END