#include "engine/core/materials/skin.h"

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {
    Render::MaterialUniforms SkinMaterial::get_uniforms() const {

    Render::MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_albedo;
    uniforms.dataSlot2 = {m_tileUV.x, m_tileUV.y, m_settings.alphaTest, m_settings.blending};
    uniforms.dataSlot3 = {m_albedoWeight, 0.0, 0.0, m_roughness};
    uniforms.dataSlot4 = {m_roughnessWeight, m_occlusion, m_occlusionWeight, m_hasAlbedoTexture};
    uniforms.dataSlot5 = {m_hasNormalTexture && m_useNormalTexture, m_hasRoughnessTexture, 0.0, m_hasAOTexture};
    uniforms.dataSlot6 = {m_hasMaskTexture, 0.0, 0.0, 0.0};
    uniforms.dataSlot7 = Vec4(0.0);
    uniforms.dataSlot8 = Vec4{m_temporalReuse, m_isReflective, 0.0f, 0.0f}; // W = Material ID

    return uniforms;
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END