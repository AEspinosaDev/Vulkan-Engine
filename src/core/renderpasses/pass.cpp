#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void BasePass::setup(std::vector<Graphics::Frame>& frames) {
    std::vector<Graphics::Attachment>        attachments;
    std::vector<Graphics::SubPassDependency> dependencies;
    setup_attachments(attachments, dependencies);

    if (!attachments.empty())
        m_renderpass = m_device->create_render_pass(m_renderpass.extent, attachments, dependencies);
    else
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
    }
    m_descriptorPool.cleanup();
}

void BasePass::create_framebuffer() {
    if (!m_initiatized || !m_isGraphical)
        return;

    const uint32_t          ATTACHMENT_COUNT = m_renderpass.attachments.size();
    std::vector<Attachment> framebufferAttachments;
    framebufferAttachments.resize(ATTACHMENT_COUNT);
    size_t presentViewIndex{0};

    for (size_t i = 0; i < ATTACHMENT_COUNT; i++)
    {
        // Create image and image view for framebuffer
        if (!m_renderpass.attachments[i].isPresentImage) // If its not default renderpass
        {
            m_renderpass.attachments[i].imageConfig.layers = m_framebufferImageDepth;
            m_renderpass.attachments[i].image              = m_device->create_image(
                {m_renderpass.extent.width, m_renderpass.extent.height, 1}, m_renderpass.attachments[i].imageConfig, false);
            m_renderpass.attachments[i].image.create_view(m_renderpass.attachments[i].imageConfig);
            m_renderpass.attachments[i].image.create_sampler(m_renderpass.attachments[i].samplerConfig);

            framebufferAttachments[i] = m_renderpass.attachments[i];
        } else
        {
            presentViewIndex = i;
        }
    }

    for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
    {
        if (m_isDefault) // If its default need swapchain PRESENT images
            framebufferAttachments[presentViewIndex].image = m_device->get_swapchain().get_present_images()[fb];

        m_framebuffers[fb] = m_device->create_framebuffer(m_renderpass, framebufferAttachments, m_framebufferImageDepth);
    }
}

void BasePass::clean_framebuffer() {
    if (!m_initiatized || !m_isGraphical)
        return;
    for (Framebuffer& fb : m_framebuffers)
        fb.cleanup();

    for (size_t i = 0; i < m_renderpass.attachments.size(); i++)
    {
        m_renderpass.attachments[i].image.cleanup();
    }
}
void BasePass::update() {
    if (!m_initiatized || !m_isGraphical)
        return;

    clean_framebuffer();
    create_framebuffer();
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
