/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GEOMETRY_PASS_H
#define GEOMETRY_PASS_H
#include <engine/core/passes/graphic_pass.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

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

    void setup_material_descriptor(IMaterial* mat, uint32_t meshIdx);

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

    void setup_uniforms(std::vector<Graphics::Frame>& frames) override;

    void setup_shader_passes() override;

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0) override;

    void link_input_attachments() override;

    void update_uniforms(uint32_t frameIndex, Scene* const scene) override;
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif