/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef ENVIROMENT_PASS_H
#define ENVIROMENT_PASS_H
#include <engine/core/passes/pass.h>
#include <engine/core/textures/textureHDR.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class EnviromentPass : public GraphicPass
{
    ColorFormatType         m_format;
    Mesh*                   m_vignette;
    Graphics::DescriptorSet m_envDescriptorSet;
    Graphics::Buffer        m_captureBuffer;
    Extent2D                m_irradianceResolution;

  public:
    EnviromentPass(Graphics::Device* ctx,
                   ColorFormatType   format,
                   Extent2D          extent,
                   uint32_t          irradianceResolution,
                   Mesh*             vignette)
        : BasePass(ctx, extent, 2, CUBEMAP_FACES, false)
        , m_vignette(vignette)
        , m_format(format)
        , m_irradianceResolution({irradianceResolution, irradianceResolution}) {
    }

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void create_framebuffer();

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void cleanup();
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif