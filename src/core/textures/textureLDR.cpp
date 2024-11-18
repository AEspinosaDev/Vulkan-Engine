#include <engine/core/textures/textureLDR.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core
{
Texture *Texture::FALLBACK_TEX = nullptr;
Texture *Texture::FALLBACK_CUBE_TEX = nullptr;
Texture *Texture::BLUE_NOISE_TEXT = nullptr;

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END