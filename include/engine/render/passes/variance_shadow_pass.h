/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VSM_PASS_H
#define VSM_PASS_H
#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

using namespace Core;
using namespace Graphics;

namespace Render {

class VarianceShadowPass final : public BaseGraphicPass
{
    /* Config  */
    ColorFormatType m_format = SRG_32F;
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
    VarianceShadowPass( const ptr<Graphics::Device>& device, const ptr<Render::GPUResourcePool>& shared, const PassLinkage<0, 2>& config, Extent2D extent, uint32_t numLights, ColorFormatType depthFormat )
        : BaseGraphicPass( device, shared, extent, 1, numLights, false, false, "SHADOWS" )
        , m_depthFormat( depthFormat ) {
        BasePass::store_attachments<0, 2>( config );
    }

    void setup_out_attachments( std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies ) override;

    void setup_uniforms( std::vector<Graphics::Frame>& frames ) override;

    void setup_shader_passes() override;

    void execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0 ) override;
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
