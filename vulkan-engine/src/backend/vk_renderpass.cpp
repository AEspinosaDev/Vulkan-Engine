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


VULKAN_ENGINE_NAMESPACE_END
