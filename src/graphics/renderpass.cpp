#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void RenderPass::begin(VkCommandBuffer &cmd, uint32_t framebufferId, VkSubpassContents subpassContents)
{
    VkRenderPassBeginInfo renderPassInfo =
        init::renderpass_begin_info(m_handle, m_extent, m_framebuffer_handles[framebufferId]);

    std::vector<VkClearValue> clearValues;
    clearValues.reserve(m_attachments.size());
    for (size_t i = 0; i < m_attachments.size(); i++)
    {
        clearValues.push_back(m_attachments[i].clearValue);
    }

    renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &renderPassInfo, subpassContents);
}

void RenderPass::end(VkCommandBuffer &cmd)
{
    vkCmdEndRenderPass(cmd);
}
void RenderPass::draw(VkCommandBuffer &cmd, Geometry *g)
{
    PROFILING_EVENT()
    RenderData *rd = get_render_data(g);
    if (rd->loadedOnGPU)
        Context::draw_geometry(cmd, rd->vbo, rd->ibo, rd->vertexCount, rd->indexCount, g->indexed());
}
void RenderPass::cleanup()
{
    if (!m_initiatized)
        return;
    vkDestroyRenderPass(m_context->device, m_handle, nullptr);
    for (auto pair : m_shaderPasses)
    {
        ShaderPass *pass = pair.second;
        pass->cleanup(m_context->device);
    }
    m_descriptorManager.cleanup();
}

void RenderPass::create_framebuffer()
{
    if (!m_initiatized)
        return;
    // Prepare data structures
    m_framebuffer_handles.resize(m_framebufferCount);

    uint32_t attachmentCount = m_attachments.size();
    std::vector<VkImageView> viewAttachments;
    viewAttachments.resize(attachmentCount);

    // If default
    size_t presentViewIndex{0};

    for (size_t i = 0; i < attachmentCount; i++)
    {
        // Create image and image view for framebuffer
        if (!m_attachments[i].isPresentImage) // If its not default renderpass
        {
            m_attachments[i].image.extent = {m_extent.width, m_extent.height, 1};
            m_attachments[i].image.config.layers = m_framebufferImageDepth;
            m_attachments[i].image.init(m_context->memory, false);

            m_attachments[i].image.create_view(m_context->device);

            m_attachments[i].image.create_sampler(m_context->device);

            viewAttachments[i] = m_attachments[i].image.view;
        }
        else
        {
            presentViewIndex = i;
        }
    }

    for (size_t fb = 0; fb < m_framebufferCount; fb++)
    {
        if (m_isDefault) // If its default need swapchain PRESENT images
            viewAttachments[presentViewIndex] = m_context->swapchain.get_present_images()[fb].view;

        VkFramebufferCreateInfo fbInfo = init::framebuffer_create_info(m_handle, m_extent);
        fbInfo.pAttachments = viewAttachments.data();
        fbInfo.attachmentCount = (uint32_t)viewAttachments.size();
        fbInfo.layers = m_framebufferImageDepth;

        if (vkCreateFramebuffer(m_context->device, &fbInfo, nullptr, &m_framebuffer_handles[fb]) != VK_SUCCESS)
        {
            throw VKException("failed to create framebuffer!");
        }
    }
}

void RenderPass::clean_framebuffer()
{
    if (!m_initiatized)
        return;
    for (VkFramebuffer &fb : m_framebuffer_handles)
        vkDestroyFramebuffer(m_context->device, fb, nullptr);

    for (size_t i = 0; i < m_attachments.size(); i++)
    {
        m_attachments[i].image.cleanup(m_context->device, m_context->memory);
    }
}
void RenderPass::update()
{
    if (!m_initiatized)
        return;

    clean_framebuffer();
    create_framebuffer();
}

VULKAN_ENGINE_NAMESPACE_END
