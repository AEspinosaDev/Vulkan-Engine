/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FORWARD_PASS_H
#define FORWARD_PASS_H
#include <engine/core/textures/texture.h>
#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

using namespace Core;
using namespace Graphics;

namespace Render {
/*
STANDARD FORWARD LIGHTING PASS
*/
class ForwardPass final : public BaseGraphicPass
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    ColorFormatType m_depthFormat;
    MSAASamples     m_aa;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
        Graphics::DescriptorSet textureDescriptor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    void setup_material_descriptor( IMaterial* mat );

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
    ForwardPass( const ptr<Graphics::Device>&     device,
                 const ptr<Render::GPUResourcePool>& shared,
                 const PassLinkage<3, 3>&         config,
                 Extent2D                         extent,
                 ColorFormatType                  colorFormat,
                 ColorFormatType                  depthFormat,
                 MSAASamples                      samples,
                 bool                             isDefault = false )
        : BaseGraphicPass( device, shared, extent, 1, 1, true, isDefault, "FORWARD" )
        , m_colorFormat( colorFormat )
        , m_depthFormat( depthFormat )
        , m_aa( samples ) {
        BasePass::store_attachments<3, 3>( config );
    }

    void setup_out_attachments( std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies ) override;

    void create_framebuffer() override;

    void setup_uniforms( std::vector<Graphics::Frame>& frames ) override;

    void setup_shader_passes() override;

    void execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0 ) override;

    void update_uniforms( uint32_t frameIndex, Scene* const scene ) override;

    void link_input_attachments() override;
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif