#include <engine/core/renderpasses/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {


void RenderPass::setup( std::vector<Graphics::Frame>& frames) {
    setup_attachments();
    m_device->create_render_pass(m_handle, m_attachments, m_dependencies);
    m_initiatized = true;
    create_framebuffer();
    setup_uniforms(frames);
    setup_shader_passes();
}

void RenderPass::cleanup() {
    if (!m_initiatized)
        return;
    m_handle.cleanup();
    for (auto pair : m_shaderPasses)
    {
        ShaderPass* pass = pair.second;
        pass->cleanup();
    }
    m_descriptorPool.cleanup();
}

void RenderPass::create_framebuffer() {
    if (!m_initiatized)
        return;

    const uint32_t          ATTACHMENT_COUNT = m_attachments.size();
    std::vector<Attachment> framebufferAttachments;
    framebufferAttachments.resize(ATTACHMENT_COUNT);
    size_t presentViewIndex{0};

    for (size_t i = 0; i < ATTACHMENT_COUNT; i++)
    {
        // Create image and image view for framebuffer
        if (!m_attachments[i].isPresentImage) // If its not default renderpass
        {
            m_attachments[i].image.extent        = {m_extent.width, m_extent.height, 1};
            m_attachments[i].image.config.layers = m_framebufferImageDepth;
            m_device->create_image(m_attachments[i].image, false);
            m_attachments[i].image.create_view();
            m_attachments[i].image.create_sampler();

            framebufferAttachments[i] = m_attachments[i];
        } else
        {
            presentViewIndex = i;
        }
    }

    for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
    {
        if (m_isDefault) // If its default need swapchain PRESENT images
            framebufferAttachments[presentViewIndex].image = m_device->get_swapchain().get_present_images()[fb];

        m_device->create_framebuffer(m_framebuffers[fb], m_handle, framebufferAttachments, m_framebufferImageDepth);
    }
}

void RenderPass::clean_framebuffer() {
    if (!m_initiatized)
        return;
    for (Framebuffer& fb : m_framebuffers)
        fb.cleanup();

    for (size_t i = 0; i < m_attachments.size(); i++)
    {
        m_attachments[i].image.cleanup();
    }
}
void RenderPass::update() {
    if (!m_initiatized)
        return;

    clean_framebuffer();
    create_framebuffer();
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
