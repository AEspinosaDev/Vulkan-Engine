#include "engine/materials/vk_unlit.h"

namespace vke
{
    Material *Material::DEBUG_MATERIAL = nullptr;
}

vke::MaterialUniforms vke::UnlitMaterial::get_uniforms() const
{
    MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_color;
    uniforms.dataSlot2 = {m_tileUV.x, m_tileUV.y, m_hasColorTexture, 0.0f};
    return uniforms;
}