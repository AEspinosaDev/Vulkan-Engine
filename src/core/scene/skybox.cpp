#include <engine/core/scene/skybox.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core
{
int Skybox::m_instanceCount = 0;
}
Graphics::Image *const Core::get_env_cubemap(Skybox *skb)
{
    return &skb->m_envCubemap;
}
Graphics::Image *const Core::get_irr_cubemap(Skybox *skb)
{
    return &skb->m_irrCubemap;
}
VULKAN_ENGINE_NAMESPACE_END