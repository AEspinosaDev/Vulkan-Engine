#include "engine/core/materials/physically_based.h"

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {
IMaterial*                 IMaterial::debugMaterial = new PhysicalMaterial(Vec4(0.5f,0.5f,0.5f,1.0));
IMaterial::GPUPayload  PhysicalMaterial::get_uniforms() const {

    IMaterial::GPUPayload  uniforms;
    uniforms.dataSlot1 = m_albedo;
    uniforms.dataSlot2 = {m_tileUV.x, m_tileUV.y, m_settings.alphaTest, m_settings.blending};
    uniforms.dataSlot3 = {m_albedoWeight, m_metalness, m_metalnessWeight, m_roughness};
    uniforms.dataSlot4 = {m_roughnessWeight, m_occlusion, m_occlusionWeight, m_hasAlbedoTexture};
    uniforms.dataSlot5 = {m_hasNormalTexture && m_useNormalTexture, m_hasRoughnessTexture, m_hasMetallicTexture, m_hasAOTexture};
    uniforms.dataSlot6 = {m_hasMaskTexture, m_maskType, m_opacityWeight, m_hasEmissiveTexture};
    uniforms.dataSlot7 = {m_emissionColor, m_emisionWeight};
    uniforms.dataSlot8 = Vec4{m_emissionIntensity, m_isReflective, 0.0f, 0.0f}; // W = Material ID

    return uniforms;
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END