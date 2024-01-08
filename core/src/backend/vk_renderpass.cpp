#include "vk_renderpass.h"

void vke::RenderPassBuilder::add_color_attachment(VkFormat& format, VkSampleCountFlagBits samples, bool stencil)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = samples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = stencil ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = stencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = samples > VK_SAMPLE_COUNT_1_BIT ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Need resolve before presenting

    colorAttachments.push_back(colorAttachment);
}

void vke::RenderPassBuilder::setup_depth_attachment(VkFormat& format, VkSampleCountFlagBits samples, bool stencil)
{
    depthAttachment.flags = 0;
    depthAttachment.format = format;
    depthAttachment.samples = samples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = stencil ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = stencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}

VkRenderPass vke::RenderPassBuilder::build_renderpass(VkDevice& device, bool colorBit, bool depthBit)
{
    std::vector<VkAttachmentReference> colorRefs;
    std::vector<VkAttachmentDescription> attachments;

    VkAttachmentReference colorAttachmentResolveRef{};
    // COLOR BIT
    if (colorBit)
    {
        for (size_t i = 0; i < colorAttachments.size(); i++)
        {
            // Color refs
            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = (uint32_t)i;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs.push_back(colorAttachmentRef);

            attachments.push_back(colorAttachments[i]);
        }
    }

    // DEPTH BIT
    VkAttachmentReference depthAttachmentRef = {};
    if (depthBit)
    {

        depthAttachmentRef.attachment = (uint32_t)colorRefs.size();
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(depthAttachment);
    }
    // RESOLVE COLOR BIT
    if (colorBit)
    
        // If main color attachment is not directly a swapchain image, for example for multisampling ...
        // THIS IS THE ONE PRESENTED TO THE SWAPCHAIN
        if (colorAttachments[0].finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            VkAttachmentDescription colorAttachmentResolve{};
            colorAttachmentResolve.format = colorAttachments[0].format;
            colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorAttachmentResolveRef.attachment = attachments.size();
            colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            attachments.push_back(colorAttachmentResolve);
        }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = colorBit ? colorAttachments.size() : 0;
    subpass.pColorAttachments = colorBit ? colorRefs.data() : VK_NULL_HANDLE;
    subpass.pDepthStencilAttachment = depthBit ? &depthAttachmentRef : VK_NULL_HANDLE;
    if (colorAttachments[0].finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkRenderPass renderPass{};

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    return renderPass;
}
