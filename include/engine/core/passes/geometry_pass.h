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
class GeometryPass : public BaseGraphicPass<3, 6>
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    ColorFormatType m_depthFormat;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    void setup_material_descriptor(IMaterial* mat);

  public:
    /*
        Input Attachments:
        -
        - Enviroment 
        - Diffuse Enviroment Irradiance
        - Sky

        Output Attachments:
        -
        - Position buffer
        - Normal buffer
        - Albedo buffer
        - Material buffer
        - Emmissive buffer
        - Depth buffer
    */
    GeometryPass(Graphics::Device*       device,
                 const PassConfig<3, 6>& config,
                 Extent2D                extent,
                 ColorFormatType         colorFormat,
                 ColorFormatType         depthFormat)
        : BaseGraphicPass(device, config, extent, 1, 1, "GEOMETRY")
        , m_colorFormat(colorFormat)
        , m_depthFormat(depthFormat) {
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                               std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void link_input_attachments();

    void update_uniforms(uint32_t frameIndex, Scene* const scene);
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif