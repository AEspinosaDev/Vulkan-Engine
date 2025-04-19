#include <engine/core/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

Core::Geometry* GraphicPass::vignette = nullptr;

void GraphicPass::setup(std::vector<Graphics::Frame>& frames) {
    std::vector<Graphics::AttachmentInfo>    attachments;
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

void GraphicPass::cleanup() {
    m_renderpass.cleanup();
    clean_framebuffer();
    BasePass::cleanup();
}

void GraphicPass::create_framebuffer() {
    if (!m_initiatized)
        return;

    for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
    {
        m_framebuffers[fb] = m_device->create_framebuffer(m_renderpass, m_imageExtent, m_framebufferImageDepth, fb);
    }
}

void GraphicPass::clean_framebuffer() {
    if (!m_initiatized )
        return;
    for (Framebuffer& fb : m_framebuffers)
        fb.cleanup();
}
void GraphicPass::resize_attachments() {
    if (!m_initiatized)
        return;

    clean_framebuffer();
    create_framebuffer();
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
