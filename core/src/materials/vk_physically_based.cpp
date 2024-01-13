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

    // float occlusion;
    // float occlusionWeight;

    //  bool hasAlbdoTexture;
    //  bool hasNormalTexture;
    //  bool hasRoughnessTexture;
    //  bool hasMetallicTexture;
    //  bool hasAOTexture;
    //  bool hasMaskTexture;
    // int maskType;
    // vec2 uvTile;

    //-----------------

    MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_albedo;
    uniforms.dataSlot2 = {m_albedoWeight, m_metalness, m_metalnessWeight, m_roughness};
    uniforms.dataSlot3 = {m_roughnessWeight, m_occlusion, m_occlusionWeight, m_hasAlbedoTexture};
    uniforms.dataSlot4 = {m_hasNormalTexture, m_hasRoughnessTexture, m_hasMetallicTexture, m_hasAOTexture};
    uniforms.dataSlot5 = {m_hasMaskTexture, m_maskType, m_tileUV.x, m_tileUV.y};

    return uniforms;
}