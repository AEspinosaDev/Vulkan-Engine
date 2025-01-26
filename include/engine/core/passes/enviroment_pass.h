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

/*
This pass does three things:
- Performs a first renderpass to convert the HDRi image into a enviroment cubemap.
- Performs a second renderpass to compute the diuffuse irradiance cubemap.
- Performs a third renderpass to compute the specular irradiance cubemap.
*/
class EnviromentPass : public GraphicPass
{
    ColorFormatType         m_format;
    Mesh*                   m_vignette;
    Graphics::DescriptorSet m_envDescriptorSet;
    Graphics::Buffer        m_captureBuffer;
    Extent2D                m_irradianceResolution;

  public:
    EnviromentPass(Graphics::Device* ctx, Mesh* vignette)
        : BasePass(ctx, {1, 1}, 2, CUBEMAP_FACES, false)
        , m_vignette(vignette)
        , m_format(SRGBA_32F)
        , m_irradianceResolution({1, 1}) {
    }

    inline uint32_t get_irradiance_resolution() const {
        return m_irradianceResolution.width;
    }
    inline void set_irradiance_resolution(uint32_t res) {
        m_irradianceResolution = {res, res};
    }

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void create_framebuffer();

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void update_framebuffer();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void cleanup();
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif