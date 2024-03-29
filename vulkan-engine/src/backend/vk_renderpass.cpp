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
    wrapper.obj = renderPass;

    return wrapper;
};

void RenderPassBuilder::create_framebuffer(VkDevice &device, VmaAllocator &memory, RenderPass &pass, VkExtent3D extent, uint32_t layers)
{
    // pass.extent = {extent.width, extent.height};

    // std::vector<VkImageView> imgAttachments;
    // for (size_t i = 0; i < attachments.size(); i++)
    // {

    //     Texture *textAttachment = new Texture();
    //     Image image;
    //     image.init(memory, attachments[i].description.format,
    //                attachments[i].viewUsage, extent, false, attachments[i].description.samples, layers);

    //     image.create_view(device, attachments[i].viewAspect, attachments[i].viewType);

    //     imgAttachments.push_back(image.view);
    //     textAttachment->m_image = image;

    //     pass.textureAttachments.push_back(textAttachment);
    // }

    // VkFramebufferCreateInfo fbInfo = init::framebuffer_create_info(pass.obj, pass.extent);
    // fbInfo.pAttachments = imgAttachments.data();
    // fbInfo.attachmentCount = (uint32_t)imgAttachments.size();
    // fbInfo.layers = layers;

    // if (vkCreateFramebuffer(device, &fbInfo, nullptr, &pass.framebuffer) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to create framebuffer!");
    // }
}

VULKAN_ENGINE_NAMESPACE_END
