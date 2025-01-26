#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void BasePass::setup(std::vector<Graphics::Frame>& frames) {
    std::vector<Graphics::AttachmentInfo>        attachments;
    std::vector<Graphics::SubPassDependency> dependencies;
    setup_attachments(attachments, dependencies);

    if (!attachments.empty())
    {
        m_renderpass = m_device->create_render_pass(attachments, dependencies);
        m_renderpass.set_debug_name(m_name.c_str());
    } else
        m_isGraphical = false;
    m_initiatized = true;

    create_framebuffer();
    setup_uniforms(frames);
    setup_shader_passes();
}

void BasePass::cleanup() {
    if (!m_initiatized)
        return;
    m_renderpass.cleanup();
    for (auto pair : m_shaderPasses)
    {
        ShaderPass* pass = pair.second;
        pass->cleanup();
        delete pass;
    }
    m_descriptorPool.cleanup();
}

void BasePass::create_framebuffer() {
    if (!m_initiatized || !m_isGraphical)
        return;
  
    for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
    {
        m_framebuffers[fb] =
            m_device->create_framebuffer(m_renderpass, m_imageExtent, m_framebufferImageDepth,fb);
    }
}

void BasePass::clean_framebuffer() {
    if (!m_initiatized || !m_isGraphical)
        return;
    for (Framebuffer& fb : m_framebuffers)
        fb.cleanup();
}
void BasePass::update_framebuffer() {
    if (!m_initiatized || !m_isGraphical)
        return;

    clean_framebuffer();
    create_framebuffer();
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
