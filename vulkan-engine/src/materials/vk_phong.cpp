#include "engine/materials/vk_phong.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

/**
 *  Alignment in shader
 *  ---------------
 *vec3 color;
 *float opacity;
 *float shininess;
 *float glossiness;
 *vec2 tileUV;
 *bool hasColorTexture;
 *bool hasOpacityTexture;
 *bool hasNormalTexture;
 *bool hasGlossinessTexture;
*/
MaterialUniforms PhongMaterial::get_uniforms() const
{

    MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_color;
    uniforms.dataSlot2 = {m_shininess, m_glossiness, m_tileUV.x, m_tileUV.y};
    uniforms.dataSlot3 = {m_hasColorTexture, m_hasNormalTexture, m_hasGlossinessTexture, 0.0f};

    return uniforms;
}
VULKAN_ENGINE_NAMESPACE_END