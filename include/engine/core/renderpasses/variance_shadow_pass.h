/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VSM_PASS_H
#define VSM_PASS_H
#include <engine/core/renderpasses/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class VarianceShadowPass : public RenderPass
{
    /* Config  */
    ColorFormatType m_format = SRG_32F;
    DepthFormatType m_depthFormat;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

  public:
    VarianceShadowPass(Graphics::Device* ctx,
                       Extent2D          extent,
                       uint32_t          framebufferCount,
                       uint32_t          numLights,
                       DepthFormatType   depthFormat)
        : RenderPass(ctx, extent, framebufferCount, numLights)
        , m_depthFormat(depthFormat) {
    }

    void setup_attachments();

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
