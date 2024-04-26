#include <engine/scene/camera.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
int Camera::m_instanceCount = 0;

void Camera::set_frustum()
{
    const float halfVSide = m_far * tanf(math::radians(m_fov) * .5f);
    const float halfHSide = halfVSide * m_aspect;
    const Vec3 frontMultFar = m_far * m_transform.forward;
    
    m_frustrum.nearFace = {m_transform.position + m_near * m_transform.forward, m_transform.forward};
    m_frustrum.farFace = {m_transform.position + frontMultFar, -m_transform.forward};
    m_frustrum.rightFace = {m_transform.position,
                            math::cross(frontMultFar - m_transform.right * halfHSide, m_transform.up)};
    m_frustrum.leftFace = {m_transform.position,
                           math::cross(m_transform.up, frontMultFar + m_transform.right * halfHSide)};
    m_frustrum.topFace = {m_transform.position,
                          math::cross(m_transform.right, frontMultFar - m_transform.up * halfVSide)};
    m_frustrum.bottomFace = {m_transform.position,
                             math::cross(frontMultFar + m_transform.up * halfVSide, m_transform.right)};
}

VULKAN_ENGINE_NAMESPACE_END