#include "engine/materials/vk_physically_based.h"

vke::MaterialUniforms vke::PhysicalBasedMaterial::get_uniforms() const
{
    // Alignment in shader
    //-----------------
    //  vec3 albedo;
    //  float opacity;
    //  float albedoWeight;

    //  float metalness;
    //  float metalnessWeight;

    //  float roughness;
    //  float roughnessWeight;

    // float emmission;
    //  vec2 tileUV;

    //  bool hasAlbdoTexture;
    //  bool hasNormalTexture;
    //  bool hasMaskTexture;
    //  bool hasEmiTexture;

    // vec4 emmissionColor;
    //-----------------

    MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_albedo;
    uniforms.dataSlot2 = {m_albedoWeight, m_metalness, m_metalnessWeight, m_roughness};
    uniforms.dataSlot3 = {m_roughnessWeight, m_emmissive, m_tileUV.x, m_tileUV.y};
    uniforms.dataSlot4 = {m_hasAlbedoTexture, m_hasNormalTexture, m_hasMaskTexture,0.0f};
    uniforms.dataSlot5 = m_emissionColor;

    return uniforms;
}