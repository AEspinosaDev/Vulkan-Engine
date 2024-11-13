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

struct Attachment {
    AttachmentType type           = AttachmentType::COLOR_ATTACHMENT;
    Image          image          = {};
    bool           isPresentImage = false;
    VkClearValue   clearValue     = {};

    VkAttachmentLoadOp  loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp  stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout       initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout       finalLayout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout       attachmentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    Attachment() {};
    Attachment(VkFormat              format,
               VkSampleCountFlagBits samples,
               VkImageLayout         final_Layout,
               VkImageLayout         attach_layout,
               VkImageUsageFlags     usage,
               AttachmentType        attachmentType = AttachmentType::COLOR_ATTACHMENT,
               VkImageAspectFlags    aspect         = VK_IMAGE_ASPECT_COLOR_BIT,
               VkImageViewType       viewType       = VK_IMAGE_VIEW_TYPE_2D,
               VkFilter              filter         = VK_FILTER_LINEAR,
               VkSamplerAddressMode  addressMode    = VK_SAMPLER_ADDRESS_MODE_REPEAT,
               VkClearValue          clearVal       = {{{0.0, 0.0, 0.0, 1.0}}})
        : finalLayout(final_Layout)
        , attachmentLayout(attach_layout)
        , clearValue(clearVal)
        , type(attachmentType) {
        image.config.format                    = format;
        image.config.usageFlags                = usage;
        image.config.samples                   = samples;
        image.viewConfig.aspectFlags           = aspect;
        image.viewConfig.viewType              = viewType;
        image.samplerConfig.filters            = filter;
        image.samplerConfig.samplerAddressMode = addressMode;
        clearValue.depthStencil.depth          = 1.0f;
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

class VulkanRenderPass
{
    VkRenderPass                   m_handle = VK_NULL_HANDLE;
    VkDevice                       m_device = VK_NULL_HANDLE;
  

  public:
    VulkanRenderPass() {
    }

    inline VkRenderPass& get_handle()  {
        return m_handle;
    }
    inline VkDevice get_device_handle() const {
        return m_device;
    };
   

    void init(VkDevice device, std::vector<Attachment> &attachments, std::vector<SubPassDependency> &dependencies);

    void cleanup();
    
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif