/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PASS_H
#define PASS_H

#include <engine/common.h>
#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

/*
Attachment info needed for Renderpasses and Framebuffers
*/
struct Attachment {
    AttachmentType type           = AttachmentType::COLOR_ATTACHMENT;
    Image          image          = {};
    ImageConfig    imageConfig    = {};
    SamplerConfig  samplerConfig  = {};
    bool           isPresentImage = false;
    VkClearValue   clearValue     = {};

    AttachmentLoadOp  loadOp           = AttachmentLoadOp::CLEAR_OP;
    AttachmentStoreOp storeOp          = AttachmentStoreOp::STORE_OP;
    AttachmentLoadOp  stencilLoadOp    = AttachmentLoadOp::DONT_CARE_OP;
    AttachmentStoreOp stencilStoreOp   = AttachmentStoreOp::DONT_CARE_OP;
    ImageLayoutType   initialLayout    = ImageLayoutType::UNDEFINED;
    ImageLayoutType   finalLayout      = ImageLayoutType::SHADER_READ_ONLY_OPTIMAL;
    ImageLayoutType   attachmentLayout = ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL;

    Attachment() {};
    Attachment(ColorFormatType   format,
               uint16_t          samples,
               ImageLayoutType   final_Layout,
               ImageLayoutType   attach_layout,
               VkImageUsageFlags usage,
               AttachmentType    attachmentType = AttachmentType::COLOR_ATTACHMENT,
               AspectType        aspect         = AspectType::COLOR,
               TextureType       viewType       = TextureType::TEXTURE_2D,
               FilterType        filter         = FilterType::LINEAR,
               AddressMode       addressMode    = AddressMode::REPEAT,
               VkClearValue      clearVal       = {{{0.0, 0.0, 0.0, 1.0}}})
        : finalLayout(final_Layout)
        , attachmentLayout(attach_layout)
        , clearValue(clearVal)
        , type(attachmentType) {
        imageConfig.format               = format;
        imageConfig.usageFlags           = usage;
        imageConfig.samples              = samples;
        imageConfig.aspectFlags          = aspect;
        imageConfig.viewType             = viewType;
        samplerConfig.filters            = filter;
        samplerConfig.samplerAddressMode = addressMode;
        clearValue.depthStencil.depth    = 1.0f;
    };
};

struct SubPassDependency {
    uint32_t             srcSubpass      = VK_SUBPASS_EXTERNAL;
    uint32_t             dstSubpass      = 0;
    VkPipelineStageFlags srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPipelineStageFlags dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkAccessFlags        srcAccessMask   = 0;
    VkAccessFlags        dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkDependencyFlags    dependencyFlags = 0; // Default flags (no special flags)

    SubPassDependency() {};
    SubPassDependency(VkPipelineStageFlags srcStage,
                      VkPipelineStageFlags dstStage,
                      VkAccessFlags        dstAccess,
                      VkDependencyFlags    deps = VK_DEPENDENCY_BY_REGION_BIT)
        : srcStageMask(srcStage)
        , dstStageMask(dstStage)
        , dstAccessMask(dstAccess)
        , dependencyFlags(deps) {
    }
};

struct VulkanRenderPass {
    VkRenderPass handle = VK_NULL_HANDLE;
    VkDevice     device = VK_NULL_HANDLE;

    Extent2D extent;

    std::vector<Graphics::Attachment>        attachments;
    std::vector<Graphics::SubPassDependency> dependencies;

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif