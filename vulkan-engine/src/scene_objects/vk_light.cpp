#include "engine/scene_objects/vk_light.h"

vke::LightUniforms vke::PointLight::get_uniforms() const
{
    LightUniforms uniforms{};
    uniforms.position = {m_transform.position.x, m_transform.position.y, m_transform.position.z, m_lighType};
    uniforms.color = {m_color.r, m_color.g, m_color.b, m_intensity};
    uniforms.dataSlot1 = {m_effectArea, m_decaying, 0.0f, m_shadow.cast};
    uniforms.dataSlot2 = {m_shadow.bias, m_shadow.enableVulkanBias, m_shadow.angleDependableBias, m_shadow.pcfKernel};
    return uniforms;
}

vke::LightUniforms vke::DirectionalLight::get_uniforms() const
{
    LightUniforms uniforms{};
    uniforms.position = {m_transform.position.x, m_transform.position.y, m_transform.position.z, m_lighType};
    uniforms.color = {m_color.r, m_color.g, m_color.b, m_intensity};
    uniforms.dataSlot1 = {m_direction.x, m_direction.y, m_direction.z, m_shadow.cast};
    uniforms.dataSlot2 = {m_shadow.bias, m_shadow.enableVulkanBias, m_shadow.angleDependableBias, m_shadow.pcfKernel};
    return uniforms;
}