#include <engine/backend/vk_renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

RenderPass RenderPassBuilder::build_renderpass(VkDevice &device)
{
    std::vector<VkAttachmentDescription> attachmentsSubDescriptions;
    attachmentsSubDescriptions.reserve(attachments.size());
    for (AttachmentDescription &attachment : attachments)
    {
        attachmentsSubDescriptions.push_back(attachment.description);
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachmentsSubDescriptions.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    VkRenderPass renderPass{};

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    RenderPass wrapper;
    wrapper.attachmentCount = attachments.size();
    wrapper.attachmentsInfo = attachments;
    wrapper.obj = renderPass;

    return wrapper;
};

void RenderPass::begin(VkCommandBuffer &cmd, RenderPass &pass, VkExtent2D extent, std::vector<VkClearValue> clearValues, uint32_t framebufferId, VkSubpassContents subpassContents)
{
    VkRenderPassBeginInfo renderPassInfo = init::renderpass_begin_info(pass.obj, extent, pass.framebuffers[framebufferId]);

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
    vkDestroyRenderPass(device, obj, nullptr);
}

void RenderPass::create_framebuffer(VkDevice &device, VmaAllocator &memory, VkExtent2D extent, uint32_t layers, uint32_t count, Swapchain *swp)
{
    extent = extent;
    framebufferCount = count;

    // Prepare data structures
    framebuffers.resize(framebufferCount);
    std::vector<VkImageView> viewAttachments;
    viewAttachments.resize(attachmentCount);
    if (imageAttachments.empty())
        imageAttachments.resize(attachmentCount);

    // Is defaulf comprobation
    bool isDefaultRenderPass{false};
    size_t presentViewIndex{0};

    for (size_t i = 0; i < attachmentCount; i++)
    {
        // Create image and image view for framebuffer
        if (attachmentsInfo[i].description.finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) //If its not default renderpass
        {
            Image image;
            image.init(memory, attachmentsInfo[i].description.format,
                       attachmentsInfo[i].viewUsage, {extent.width, extent.height, 1}, false, attachmentsInfo[i].description.samples, layers);
            image.create_view(device, attachmentsInfo[i].viewAspect, attachmentsInfo[i].viewType);
            viewAttachments[i] = image.view;

            imageAttachments[i] = image;
        }
        else
        {
            isDefaultRenderPass = true;
            presentViewIndex = i;
        }
    }

    for (size_t fb = 0; fb < count; fb++)
    {
        if (isDefaultRenderPass) //If its default need swapchain PRESENT images
            viewAttachments[presentViewIndex] = swp->get_present_images()[fb].view;

        VkFramebufferCreateInfo fbInfo = init::framebuffer_create_info(obj, extent);
        fbInfo.pAttachments = viewAttachments.data();
        fbInfo.attachmentCount = (uint32_t)viewAttachments.size();
        fbInfo.layers = layers;

        if (vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffers[fb]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void RenderPass::clean_framebuffer(VkDevice &device, VmaAllocator &memory, bool destroyImageSamplers)
{
    for (VkFramebuffer &fb : framebuffers)
        vkDestroyFramebuffer(device, fb, nullptr);

    for (size_t i = 0; i < attachmentCount; i++)
    {
        imageAttachments[i].cleanup(device, memory, destroyImageSamplers);
    }
}
VULKAN_ENGINE_NAMESPACE_END
