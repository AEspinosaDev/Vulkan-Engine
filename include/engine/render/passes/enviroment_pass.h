/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef ENVIROMENT_PASS_H
#define ENVIROMENT_PASS_H
#include <engine/core/textures/texture_template.h>
#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

/*
This pass does three things:
- Performs a first renderpass to convert the HDRi image into a enviroment cubemap.
- Performs a second renderpass to compute the diuffuse irradiance cubemap.
- Performs a third renderpass to compute the specular irradiance cubemap.
*/
class EnviromentPass final : public BaseGraphicPass
{
    ColorFormatType         m_format;
    Graphics::DescriptorSet m_envDescriptorSet;
    Graphics::Buffer        m_captureBuffer;
    Extent2D                m_irradianceResolution;

    Core::SkySettings m_settings;

  public:
    /*

            Output Attachments:
            -
            - Enviroment Cubemap
            - Diffuse Irradiance Cubemap

        */
    EnviromentPass(const ptr<Graphics::Device>& device, const PassLinkage<1, 2>& config)
        : BaseGraphicPass(device, {1, 1}, 2, CUBEMAP_FACES, false, false, "ENVIROMENT")
        , m_format(SRGBA_32F)
        , m_irradianceResolution({1, 1}) {
        BasePass::store_attachments<1, 2>(config);
    }

    inline uint32_t get_irradiance_resolution() const {
        return m_irradianceResolution.width;
    }
    inline void set_irradiance_resolution(uint32_t res) {
        m_irradianceResolution = {res, res};
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) override;

    void create_framebuffer() override;

    void setup_uniforms(std::vector<Graphics::Frame>& frames, const Render::Resources& shared) override;

    void setup_shader_passes() override;

    void resize_attachments() override;

    void update_uniforms(const Render::RenderView& view, const Render::Resources& shared) override;

    void execute(const Render::RenderView& view, const Render::Resources& shared) override;

    void link_input_attachments() override;

    void cleanup() override;

    inline void set_sky_settings(const Core::SkySettings &sky) {
        m_settings = sky;
    }
};

} // namespace Render
VULKAN_ENGINE_NAMESPACE_END

#endif