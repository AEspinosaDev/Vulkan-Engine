#include <engine/renderpasses/guipass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void GUIPass::init(VkDevice &device)
{
    bool multisampled = m_samples > VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentDescription colorAttachment = init::attachment_description(static_cast<VkFormat>(m_colorFormat),
                                                                           multisampled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                                                           m_samples);
    m_attachments.push_back(Attachment({colorAttachment,
                                        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                        VK_IMAGE_ASPECT_COLOR_BIT}));

    if (multisampled)
    {
        VkAttachmentDescription resolveAttachment = init::attachment_description(static_cast<VkFormat>(m_colorFormat), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_attachments.push_back(Attachment({resolveAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}));
    }

  

    VkSubpassDependency colorDep = init::subpass_dependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                            0,
                                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
 

    VkAttachmentReference colorRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkSubpassDescription defaultSubpass = init::subpass_description(1, &colorRef,{});
    if (multisampled)
    {
        VkAttachmentReference resolveRef = init::attachment_reference(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        defaultSubpass.pResolveAttachments = &resolveRef;
    }

    std::vector<VkSubpassDescription> subpasses = {defaultSubpass};
    std::vector<VkSubpassDependency> dependencies = {colorDep};

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
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
    // // Build renderpass
    // build(device, {defaultSubpass}, {colorDep, depthDep});
}
void GUIPass::init_shaderpasses(VkDevice &device, DescriptorManager &descriptorManager)
{
}
void GUIPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t framebufferIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd, framebufferIndex);

    RenderPass::set_viewport(cmd, m_extent);

    if (m_gui)
        m_gui->upload_draw_data(cmd);

    end(cmd);
}

VULKAN_ENGINE_NAMESPACE_END