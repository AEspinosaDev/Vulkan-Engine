/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TONEMAPPING_PASS_H
#define TONEMAPPING_PASS_H
#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {
/*
Postprocess pass for tonemapping
*/
class TonemappingPass : public GraphicPass
{
    ColorFormatType         m_colorFormat;
    Mesh*                   m_vignette;
    Graphics::DescriptorSet m_imageDescriptorSet;

  public:
    TonemappingPass(Graphics::Device* ctx,
                    Extent2D          extent,
                    uint32_t          framebufferCount,
                    ColorFormatType   colorFormat,
                    Mesh*             vignette,
                    bool              isDefault = true)
        : BasePass(ctx, extent, framebufferCount, 1, isDefault)
        , m_colorFormat(colorFormat)
        , m_vignette(vignette) {
    }

    void setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void connect_to_previous_images(std::vector<Graphics::Image> images);
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif