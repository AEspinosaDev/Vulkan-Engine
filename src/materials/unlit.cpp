#include "engine/materials/unlit.h"

VULKAN_ENGINE_NAMESPACE_BEGIN
Material *Material::DEBUG_MATERIAL = nullptr;

MaterialUniforms UnlitMaterial::get_uniforms() const
{
    MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_color;
    uniforms.dataSlot2 = {m_tileUV.x, m_tileUV.y, m_hasColorTexture, 0.0f};
    return uniforms;
}
VULKAN_ENGINE_NAMESPACE_END