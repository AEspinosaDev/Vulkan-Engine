/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H
#include <engine/core/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class ShadowPass final : public BaseGraphicPass
{
    /* Config  */
    ColorFormatType m_depthFormat;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

  public:
    /*

              Output Attachments:
              -
              - Shadow Maps 2D Array
              - Depth 2D Array

          */
    ShadowPass(const ptr<Graphics::Device>& device, const PassLinkage<0, 2>& config, Extent2D extent, uint32_t numLights, ColorFormatType depthFormat)
        : BaseGraphicPass(device, extent, 1, numLights, false, false, "SHADOWS")
        , m_depthFormat(depthFormat) {
        BasePass::store_attachments<0, 2>(config);
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) override;

    void setup_uniforms(std::vector<Graphics::Frame>& frames) override;

    void setup_shader_passes() override;

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0) override;
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
