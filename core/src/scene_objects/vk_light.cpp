#include "engine/scene_objects/vk_light.h"

vke::LightUniforms vke::PointLight::get_uniforms() const
{
    LightUniforms uniforms{};
    uniforms.position = {m_transform.position.x, m_transform.position.y, m_transform.position.z, 0.0f};
    uniforms.color = {m_color.r, m_color.g, m_color.b, m_intensity};
    uniforms.dataSlot = {m_effectArea, m_decaying, 0.0f, 0.0f};
    return uniforms;
}

vke::LightUniforms vke::DirectionalLight::get_uniforms() const
{
    LightUniforms uniforms{};
    uniforms.position = {m_transform.position.x, m_transform.position.y, m_transform.position.z, 1.0f};
    uniforms.color = {m_color.r, m_color.g, m_color.b, m_intensity};
    uniforms.dataSlot = {m_direction.x, m_direction.y, m_direction.z, 0.0f};
    return uniforms;
}
