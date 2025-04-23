#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

Core::Geometry*              BasePass::vignette = nullptr;
std::vector<Graphics::Image> BasePass::attachmentPool;

BasePass::BasePass(Graphics::Device* device,
                   Extent2D          extent,
                   bool              isGraphical,
                   bool              isResizeable,
                   bool              isDefault,
                   std::string       name)
    : m_device(device)
    , m_name(name)
    , m_isDefault(isDefault)
    , m_imageExtent(extent)
    , m_isGraphical(isGraphical)
    , m_isResizeable(isResizeable) {
}

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

    // Delete internal intermediate attachmetns (if any)
    for (size_t i = 0; i < m_interAttachments.size(); i++)
    {
        m_interAttachments[i].cleanup();
    }

    m_descriptorPool.cleanup();
    m_initiatized = false;
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
