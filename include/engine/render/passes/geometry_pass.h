/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GEOMETRY_PASS_H
#define GEOMETRY_PASS_H
#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

/*
DEFERRED RENDERING GEOMETRY PASS
*/
class GeometryPass final : public BaseGraphicPass
{
    /*Setup*/
    ColorFormatType m_depthFormat;
    ColorFormatType m_floatFormat;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
        Graphics::DescriptorSet textureDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

  public:
    /*
        Input Attachments:
        -
        - Enviroment
        - Diffuse Enviroment Irradiance
        - Sky

        Output Attachments:
        -
        - Normal buffer
        - Albedo buffer
        - Material buffer
        - Velocity + Emissive buffer
        - Depth buffer
    */
    GeometryPass(const ptr<Graphics::Device>& device,
                 const PassLinkage<3, 5>&     config,
                 Extent2D                     extent,
                 ColorFormatType              floatingPointFormat,
                 ColorFormatType              depthFormat)
        : BaseGraphicPass(device, extent, 1, 1, true, false, "GEOMETRY")
        , m_depthFormat(depthFormat)
        , m_floatFormat(floatingPointFormat) {
        BasePass::store_attachments<3, 5>(config);
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) override;

    void setup_uniforms(std::vector<Graphics::Frame>& frames, const Render::Resources& shared) override;

    void setup_shader_passes() override;

    void execute(const Render::RenderView& view, const Render::Resources& shared) override;

    void link_input_attachments() override;

    void update_uniforms(const Render::RenderView& view, const Render::Resources& shared) override;
};
} // namespace Render
VULKAN_ENGINE_NAMESPACE_END

#endif