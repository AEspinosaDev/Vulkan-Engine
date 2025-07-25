#include <engine/core/scene/light.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {
uint16_t Light::m_nonRaytraceCount         = 0;
int      PointLight::m_instanceCount       = 0;
int      DirectionalLight::m_instanceCount = 0;

Light::GPUPayload PointLight::get_uniforms(Mat4 cameraView) const {
    Light::GPUPayload uniforms{};
    // Transform to camera view for shader
    uniforms.position      = cameraView * Vec4(m_transform.position, 1.0f);
    uniforms.position.w    = (float)m_lighType;
    uniforms.worldPosition = Vec4(m_transform.position, 1.0f);
    uniforms.color         = {m_color.r, m_color.g, m_color.b, m_intensity};
    uniforms.dataSlot1     = {m_effectArea, m_area, (int)m_shadow.type, m_shadow.cast};
    uniforms.dataSlot2     = {m_shadow.type == ShadowType::VSM_SHADOW ? m_shadow.bleeding : m_shadow.bias,
                          m_shadow.kernelRadius,
                          m_shadow.angleDependableBias,
                          m_shadow.softness};
    if (m_shadow.type == ShadowType::RAYTRACED_SHADOW)
        uniforms.dataSlot2 = {m_transform.position, m_shadow.raySamples};
    return uniforms;
}

Light::GPUPayload DirectionalLight::get_uniforms(Mat4 cameraView) const {

    Light::GPUPayload uniforms{};
    // Transform to camera view for shader

    uniforms.position      = cameraView * Vec4(m_direction, 0.0f);
    uniforms.position.w    = (float)m_lighType;
    uniforms.worldPosition = Vec4(m_direction, 0.0f);
    uniforms.color         = {m_color.r, m_color.g, m_color.b, m_intensity};
    uniforms.dataSlot1     = {0.0f, m_area, (int)m_shadow.type, m_shadow.cast};
    uniforms.dataSlot2     = {m_shadow.type == ShadowType::VSM_SHADOW ? m_shadow.bleeding : m_shadow.bias,
                          m_shadow.kernelRadius,
                          m_shadow.angleDependableBias,
                          m_shadow.softness};
    if (m_shadow.type == ShadowType::RAYTRACED_SHADOW)
        uniforms.dataSlot2 = {m_direction, m_shadow.raySamples};
    return uniforms;
}
Vec3 DirectionalLight::get_sun_direction(float elevationDeg, float rotationDeg) {
    Vec3  sunDirection;
    float sunElevationRad = math::radians(elevationDeg);
    float sunRotationRad  = math::radians(rotationDeg);
    sunDirection.x        = -cos(sunElevationRad) * sin(sunRotationRad);
    sunDirection.y        = sin(sunElevationRad);
    sunDirection.z        = -cos(sunElevationRad) * cos(sunRotationRad);
    return math::normalize(sunDirection);
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END