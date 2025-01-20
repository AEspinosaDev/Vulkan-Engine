/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <engine/common.h>
#include <engine/graphics/extensions.h>
#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

/*
Attachment info needed for Renderpasses and Framebuffers
*/
struct AttachmentInfo {
    //Image Info
    AttachmentType type          = AttachmentType::COLOR_ATTACHMENT;
    ImageConfig    imageConfig   = {};
    SamplerConfig  samplerConfig = {};
    ClearValue     clearValue    = {};
    bool           isDefault     = false;
    
    //Renderpass Info
    AttachmentLoadOp  loadOp           = ATTACHMENT_LOAD_OP_CLEAR;
    AttachmentStoreOp storeOp          = ATTACHMENT_STORE_OP_STORE;
    AttachmentLoadOp  stencilLoadOp    = ATTACHMENT_LOAD_OP_DONT_CARE;
    AttachmentStoreOp stencilStoreOp   = ATTACHMENT_STORE_OP_DONT_CARE;
    ImageLayout       initialLayout    = LAYOUT_UNDEFINED;
    ImageLayout       finalLayout      = LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImageLayout       attachmentLayout = LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    AttachmentInfo() {};
    AttachmentInfo(ColorFormatType     format,
                   uint16_t            samples,
                   ImageLayout         final_Layout,
                   ImageLayout         attach_layout,
                   ImageUsageFlags     usage,
                   AttachmentType      attachmentType = AttachmentType::COLOR_ATTACHMENT,
                   ImageAspect         aspect         = ASPECT_COLOR,
                   TextureTypeFlagBits viewType       = TEXTURE_2D,
                   FilterType          filter         = FILTER_LINEAR,
                   AddressMode         addressMode    = ADDRESS_MODE_CLAMP_TO_EDGE,
                   ClearValue          clearVal       = {{{0.0, 0.0, 0.0, 1.0}}})
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
    uint32_t              srcSubpass      = VK_SUBPASS_EXTERNAL;
    uint32_t              dstSubpass      = 0;
    PipelineStage         srcStageMask    = STAGE_COLOR_ATTACHMENT_OUTPUT;
    PipelineStage         dstStageMask    = STAGE_COLOR_ATTACHMENT_OUTPUT;
    AccessFlags           srcAccessMask   = AccessFlags::ACCESS_NONE;
    AccessFlags           dstAccessMask   = AccessFlags::ACCESS_COLOR_ATTACHMENT_WRITE;
    SubPassDependencyType dependencyFlags = SUBPASS_DEPENDENCY_NONE;

    SubPassDependency() {};
    SubPassDependency(PipelineStage         srcStage,
                      PipelineStage         dstStage,
                      AccessFlags           dstAccess,
                      SubPassDependencyType deps = SUBPASS_DEPENDENCY_BY_REGION)
        : srcStageMask(srcStage)
        , dstStageMask(dstStage)
        , dstAccessMask(dstAccess)
        , dependencyFlags(deps) {
    }
};

struct RenderPass {
    VkRenderPass handle = VK_NULL_HANDLE;
    VkDevice     device = VK_NULL_HANDLE;

    std::vector<Graphics::AttachmentInfo>    attachmentsInfo;
    std::vector<Graphics::SubPassDependency> dependenciesInfo;

    const char* debbugName = nullptr;

    void set_debug_name(const char* name);
    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif