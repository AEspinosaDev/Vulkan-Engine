#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Render {

void BaseGraphicPass::setup(std::vector<Graphics::Frame>& frames) {
    std::vector<Graphics::AttachmentConfig>  attachments;
    std::vector<Graphics::SubPassDependency> dependencies;
    setup_out_attachments(attachments, dependencies);

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
void BaseGraphicPass::cleanup() {
    m_renderpass.cleanup();
    clean_framebuffer();

    BasePass::cleanup();
}
void BaseGraphicPass::create_framebuffer() {
    CHECK_INITIALIZATION()
    for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
    {
        m_framebuffers[fb] =
            m_device->create_framebuffer(m_renderpass, m_outAttachments, m_imageExtent, m_framebufferImageDepth, fb);
    }
}
void BaseGraphicPass::clean_framebuffer() {
    CHECK_INITIALIZATION()
    for (Graphics::Framebuffer& fb : m_framebuffers)
        fb.cleanup();

    for (size_t i = 0; i < m_outAttachments.size(); i++)
    {
        m_outAttachments[i]->cleanup();
    }
}
void BaseGraphicPass::resize_attachments() {
    CHECK_INITIALIZATION()
    for (size_t i = 0; i < m_interAttachments.size(); i++)
    {
        m_interAttachments[i].cleanup();
    }
    clean_framebuffer();
    create_framebuffer();
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END