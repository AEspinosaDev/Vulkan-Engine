#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void RenderPass::build(VkDevice &device,
                       std::vector<VkSubpassDescription> subpasses,
                       std::vector<VkSubpassDependency> dependencies)
{
    std::vector<VkAttachmentDescription> attachmentsSubDescriptions;
    attachmentsSubDescriptions.reserve(m_attachments.size());
    for (Attachment &attachment : m_attachments)
    {
        attachmentsSubDescriptions.push_back(attachment.description.description);
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)attachmentsSubDescriptions.size();
    renderPassInfo.pAttachments = attachmentsSubDescriptions.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_obj) != VK_SUCCESS)
    {
        throw VKException("failed to create renderpass!");
    }

    m_initiatized = true;
}

void RenderPass::begin(VkCommandBuffer &cmd, uint32_t framebufferId, VkSubpassContents subpassContents)
{
    VkRenderPassBeginInfo renderPassInfo = init::renderpass_begin_info(m_obj, m_extent, m_framebuffers[framebufferId]);

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
void RenderPass::cleanup(VkDevice &device)
{
    if (!m_initiatized)
        return;
    vkDestroyRenderPass(device, m_obj, nullptr);
    for (auto pair : m_shaderPasses)
    {
        ShaderPass *pass = pair.second;
        pass->cleanup(device);
    }
}

void RenderPass::create_framebuffer(VkDevice &device, VmaAllocator &memory, Swapchain *swp)
{
    if (!m_initiatized)
        return;
    // Prepare data structures
    m_framebuffers.resize(m_framebufferCount);

    uint32_t attachmentCount = m_attachments.size();
    std::vector<VkImageView> viewAttachments;
    viewAttachments.resize(attachmentCount);

    // If default
    size_t presentViewIndex{0};

    for (size_t i = 0; i < attachmentCount; i++)
    {
        // Create image and image view for framebuffer
        if (m_attachments[i].description.description.finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) // If its not default renderpass
        {
            Image image;
            image.init(memory, m_attachments[i].description.description.format,
                       m_attachments[i].description.viewUsage, {m_extent.width, m_extent.height, 1}, false, m_attachments[i].description.description.samples, m_framebufferImageDepth);
            image.create_view(device, m_attachments[i].description.viewAspect, m_attachments[i].description.viewType);
            viewAttachments[i] = image.view;

            m_attachments[i].image = image;
        }
        else
        {
            presentViewIndex = i;
        }
    }

    for (size_t fb = 0; fb < m_framebufferCount; fb++)
    {
        if (m_isResolve) // If its default need swapchain PRESENT images
            viewAttachments[presentViewIndex] = swp->get_present_images()[fb].view;

        VkFramebufferCreateInfo fbInfo = init::framebuffer_create_info(m_obj, m_extent);
        fbInfo.pAttachments = viewAttachments.data();
        fbInfo.attachmentCount = (uint32_t)viewAttachments.size();
        fbInfo.layers = m_framebufferImageDepth;

        if (vkCreateFramebuffer(device, &fbInfo, nullptr, &m_framebuffers[fb]) != VK_SUCCESS)
        {
            throw VKException("failed to create framebuffer!");
        }
    }
}

void RenderPass::clean_framebuffer(VkDevice &device, VmaAllocator &memory, bool destroyImageSamplers)
{
    if (!m_initiatized)
        return;
    for (VkFramebuffer &fb : m_framebuffers)
        vkDestroyFramebuffer(device, fb, nullptr);

    for (size_t i = 0; i < m_attachments.size(); i++)
    {
        m_attachments[i].image.cleanup(device, memory, destroyImageSamplers);
    }
}
void RenderPass::update(VkDevice &device, VmaAllocator &memory, uint32_t layers, uint32_t count, Swapchain *swp)
{
    m_framebufferCount = count;
    m_framebufferImageDepth = layers;

    if (!m_initiatized)
        return;

    clean_framebuffer(device, memory, false);
    create_framebuffer(device, memory, swp);
}

void RenderPass::set_viewport(VkCommandBuffer &cmd, VkExtent2D extent, float minDepth, float maxDepth, float x, float y, int offsetX, int offsetY)
{
    // Viewport setup
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {offsetX, offsetY};
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

VULKAN_ENGINE_NAMESPACE_END
