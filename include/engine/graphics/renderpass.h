/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <engine/common.h>
#include <engine/graphics/extensions.h>
#include <engine/graphics/framebuffer.h>
#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

// Defines the attachment
struct RenderTargetInfo {
    Extent2D        extent;
    FormatType      format;
    ImageUsageFlags usage;
    ImageLayout     initialLayout;
    ImageLayout     finalLayout;
    ClearValue      clearValue = {};
    bool            load       = false; // If not load is cleaned (stencil + color)
    bool            store      = true;
    uint32_t        samples    = 1;
    uint32_t        layers     = 1;
    uint32_t        mipmaps    = 1;
    bool            resolve    = false;
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
    SubPassDependency( PipelineStage srcStage, PipelineStage dstStage, AccessFlags dstAccess, SubPassDependencyType deps = SUBPASS_DEPENDENCY_BY_REGION )
        : srcStageMask( srcStage )
        , dstStageMask( dstStage )
        , dstAccessMask( dstAccess )
        , dependencyFlags( deps ) {
    }
};

struct RenderPass {
    VkRenderPass handle = VK_NULL_HANDLE;
    VkDevice     device = VK_NULL_HANDLE;

    std::vector<RenderTargetInfo>  targetInfos;
    std::vector<SubPassDependency> dependencies;

    std::vector<Framebuffer*> fbos;

    const char* debbugName = nullptr;

    void set_debug_name( const char* name );
    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif