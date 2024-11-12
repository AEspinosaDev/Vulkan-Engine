#include <engine/graphics/vk_renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {
void VulkanRenderPass::init(VkDevice                        device,
                            std::vector<Attachment>&        attachments,
                            std::vector<SubPassDependency>& dependencies) {

    // ATTACHMENT SETUP ----------------------------------
    m_device = device;

    std::vector<VkAttachmentDescription> attachmentsInfo;
    attachmentsInfo.resize(attachments.size(), {});

    std::vector<VkAttachmentReference> colorAttachmentRefs;
    bool                               hasDepthAttachment = false;
    VkAttachmentReference              depthAttachmentRef;
    bool                               hasResolveAttachment = false;
    VkAttachmentReference              resolveAttachemtRef;

    for (size_t i = 0; i < attachmentsInfo.size(); i++)
    {
        attachmentsInfo[i].format         = attachments[i].image.config.format;
        attachmentsInfo[i].samples        = attachments[i].image.config.samples;
        attachmentsInfo[i].loadOp         = attachments[i].loadOp;
        attachmentsInfo[i].storeOp        = attachments[i].storeOp;
        attachmentsInfo[i].stencilLoadOp  = attachments[i].stencilLoadOp;
        attachmentsInfo[i].stencilStoreOp = attachments[i].stencilStoreOp;
        attachmentsInfo[i].initialLayout  = attachments[i].initialLayout;
        attachmentsInfo[i].finalLayout    = attachments[i].finalLayout;

        switch (attachments[i].type)
        {
        case AttachmentType::COLOR_ATTACHMENT:
            colorAttachmentRefs.push_back(Init::attachment_reference(i, attachments[i].attachmentLayout));
            break;
        case AttachmentType::DEPTH_ATTACHMENT:
            hasDepthAttachment = true;
            depthAttachmentRef = Init::attachment_reference(i, attachments[i].attachmentLayout);
            break;
        case AttachmentType::RESOLVE_ATTACHMENT:
            hasResolveAttachment = true;
            resolveAttachemtRef  = Init::attachment_reference(i, attachments[i].attachmentLayout);
            break;
        }
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
    subpass.pColorAttachments    = colorAttachmentRefs.data();
    if (hasDepthAttachment)
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    if (hasResolveAttachment)
        subpass.pResolveAttachments = &resolveAttachemtRef;

    // SUBPASS DEPENDENCIES SETUP ----------------------------------

    std::vector<VkSubpassDependency> subpassDependencies;
    subpassDependencies.resize(dependencies.size(), {});

    // Depdencies
    for (size_t i = 0; i < subpassDependencies.size(); i++)
    {
        subpassDependencies[i].srcSubpass      = dependencies[i].srcSubpass;
        subpassDependencies[i].dstSubpass      = dependencies[i].dstSubpass;
        subpassDependencies[i].srcStageMask    = dependencies[i].srcStageMask;
        subpassDependencies[i].dstStageMask    = dependencies[i].dstStageMask;
        subpassDependencies[i].srcAccessMask   = dependencies[i].srcAccessMask;
        subpassDependencies[i].dstAccessMask   = dependencies[i].dstAccessMask;
        subpassDependencies[i].dependencyFlags = dependencies[i].dependencyFlags;
    }

    // Creation
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
    renderPassInfo.pAttachments    = attachmentsInfo.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassInfo.pDependencies   = subpassDependencies.data();

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        new VKFW_Exception("failed to create renderpass!");
    }
}
void VulkanRenderPass::cleanup() {
    if (m_handle != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(m_device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END