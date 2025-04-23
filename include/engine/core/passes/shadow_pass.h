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

class ShadowPass : public BaseGraphicPass<0, 2>
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
    ShadowPass(Graphics::Device*       device,
               const PassConfig<0, 2>& config,
               Extent2D                extent,
               uint32_t                numLights,
               ColorFormatType         depthFormat)
        : BaseGraphicPass(device, config, extent, 1, numLights, "SHADOWS")
        , m_depthFormat(depthFormat) {
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                               std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
