#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

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
void RenderPass::cleanup(VkDevice &device, VmaAllocator &memory)
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
        if (!m_attachments[i].isPresentImage) // If its not default renderpass
        {
            m_attachments[i].image.init(memory,
                                        m_attachments[i].image.format,
                                        m_attachments[i].viewUsage,
                                        {m_extent.width, m_extent.height, 1},
                                        false,
                                        m_attachments[i].samples,
                                        m_framebufferImageDepth);

            m_attachments[i].image.create_view(device,
                                               m_attachments[i].viewAspect,
                                               m_attachments[i].viewType);

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
void RenderPass::update(VkDevice &device, VmaAllocator &memory, Swapchain *swp)
{
   

    if (!m_initiatized)
        return;

    clean_framebuffer(device, memory, true);
    create_framebuffer(device, memory, swp);
}


VULKAN_ENGINE_NAMESPACE_END
