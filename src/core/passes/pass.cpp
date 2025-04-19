#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void BasePass::setup(std::vector<Graphics::Frame>& frames) {
  
    m_isGraphical = false;
    m_initiatized = true;

    setup_uniforms(frames);
    setup_shader_passes();
}

void BasePass::cleanup() {
    if (!m_initiatized)
        return;
    for (auto pair : m_shaderPasses)
    {
        ShaderPass* pass = pair.second;
        pass->cleanup();
        delete pass;
    }
    m_descriptorPool.cleanup();
    m_initiatized = false;
}


} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
