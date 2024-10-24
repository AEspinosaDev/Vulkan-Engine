/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H
#include <engine/core/renderpasses/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

class ShadowPass : public RenderPass
{
    /* Config  */
    DepthFormatType m_depthFormat;

    /*Descriptors*/
    struct FrameDescriptors
    {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

  public:
    ShadowPass(Graphics::Context *ctx, Extent2D extent, uint32_t framebufferCount, uint32_t numLights,
               DepthFormatType depthFormat)
        : RenderPass(ctx, extent, framebufferCount, numLights), m_depthFormat(depthFormat)
    {
    }

    void init();

    void create_descriptors();

    void create_graphic_pipelines();

    void render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
