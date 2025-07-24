#include "engine/core/materials/unlit.h"

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {


IMaterial::GPUPayload  UnlitMaterial::get_uniforms() const {
    IMaterial::GPUPayload  uniforms;
    uniforms.dataSlot1 = m_color;
    uniforms.dataSlot2 = {m_tileUV.x, m_tileUV.y, m_settings.alphaTest, m_hasColorTexture};
    uniforms.dataSlot8 = Vec4{1.0};
    return uniforms;
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END