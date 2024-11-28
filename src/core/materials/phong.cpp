#include "engine/core/materials/phong.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{
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
Graphics::MaterialUniforms PhongMaterial::get_uniforms() const
{

    Graphics::MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_color;
    uniforms.dataSlot2 = {m_shininess, m_glossiness, m_tileUV.x, m_tileUV.y};
    uniforms.dataSlot3 = {m_hasColorTexture, m_hasNormalTexture, m_hasGlossinessTexture, 0.0f};
    uniforms.dataSlot8 = Vec4{0.3f}; //Shader ID

    return uniforms;
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END