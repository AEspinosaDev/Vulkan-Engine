/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FORWARD_PASS_H
#define FORWARD_PASS_H
#include <engine/core/passes/graphic_pass.h>
#include <engine/core/resource_manager.h>
#include <engine/core/textures/texture.h>
#include <engine/core/textures/textureLDR.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
STANDARD FORWARD LIGHTING PASS
*/
class ForwardPass : public BaseGraphicPass<3, 3>
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    ColorFormatType m_depthFormat;
    MSAASamples     m_aa;

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
        - Lighting
        - Bright Lighting (HDR)
        - Depth 
    */
    ForwardPass(Graphics::Device* device,
                const PassConfig<3, 3>&  config,
                Extent2D          extent,
                ColorFormatType   colorFormat,
                ColorFormatType   depthFormat,
                MSAASamples       samples)
        : BaseGraphicPass(device, config, extent, 1, 1, "FORWARD")
        , m_colorFormat(colorFormat)
        , m_depthFormat(depthFormat)
        , m_aa(samples) {
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void link_input_attachments();

};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif