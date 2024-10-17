#include "engine/core/materials/unlit.h"

VULKAN_ENGINE_NAMESPACE_BEGIN
Material *Material::DEBUG_MATERIAL = nullptr;

graphics::MaterialUniforms UnlitMaterial::get_uniforms() const
{
    graphics::MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_color;
    uniforms.dataSlot2 = {m_tileUV.x, m_tileUV.y, m_settings.alphaTest, m_hasColorTexture};
    return uniforms;
}
VULKAN_ENGINE_NAMESPACE_END