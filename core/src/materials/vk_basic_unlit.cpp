#include "engine/materials/vk_basic_unlit.h"

vke::MaterialUniforms vke::BasicUnlitMaterial::get_uniforms() const
{
    MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_color;
    uniforms.dataSlot2 = {m_tileUV.x, m_tileUV.y, m_hasColorTexture, 0.0f};
    return uniforms;
}