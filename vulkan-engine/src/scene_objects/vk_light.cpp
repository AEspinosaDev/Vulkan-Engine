#include <engine/scene_objects/vk_light.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

int PointLight::m_instanceCount = 0;
int DirectionalLight::m_instanceCount = 0;

LightUniforms PointLight::get_uniforms(Mat4 cameraView) const
{
    LightUniforms uniforms{};
    // Transform to camera view for shader
    uniforms.position = cameraView * Vec4(m_transform.position, 1.0f);
    uniforms.position.w = (float)m_lighType;
    uniforms.color = {m_color.r, m_color.g, m_color.b, m_intensity};
    uniforms.dataSlot1 = {m_effectArea, m_decaying, 0.0f, m_shadow.cast};
    uniforms.dataSlot2 = {m_shadow.bias, m_shadow.enableVulkanBias, m_shadow.angleDependableBias, m_shadow.pcfKernel};
    return uniforms;
}

LightUniforms DirectionalLight::get_uniforms(Mat4 cameraView) const
{
    LightUniforms uniforms{};
    // Transform to camera view for shader
    uniforms.position = {m_transform.position.x, m_transform.position.y, m_transform.position.z, m_lighType};
    uniforms.position = cameraView * Vec4(m_transform.position, 1.0f);
    uniforms.position.w = (float)m_lighType;
    uniforms.color = {m_color.r, m_color.g, m_color.b, m_intensity};
    // Transform to camera view for shader
    uniforms.dataSlot1 = cameraView * Vec4(m_direction, 0.0f);
    uniforms.dataSlot1.w = m_shadow.cast;
    uniforms.dataSlot2 = {m_shadow.bias, m_shadow.enableVulkanBias, m_shadow.angleDependableBias, m_shadow.pcfKernel};
    return uniforms;
}
VULKAN_ENGINE_NAMESPACE_END