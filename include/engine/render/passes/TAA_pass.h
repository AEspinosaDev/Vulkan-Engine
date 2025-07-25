/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TAA_PASS_H
#define TAA_PASS_H
#include <engine/render/passes/postprocess_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

using namespace Core;
using namespace Graphics;

namespace Render {

/*
Tempopral Filtering Pass.
*/
class TAAPass final : public PostProcessPass<2, 1>
{
public:
    /*

               Input Attachments:
               -
               - Color
               - Velocity Buffer

               Output Attachments:
               -
               - Filtered Color
               - Prev Filtered Color

           */
    TAAPass( const ptr<Graphics::Device>& device, const ptr<Render::GPUResourcePool>& shared, const PassLinkage<2, 1>& linkage, Extent2D extent, ColorFormatType colorFormat, bool isDefault = true )
        : PostProcessPass( device, shared, linkage, extent, colorFormat, GET_RESOURCE_PATH( "shaders/aa/taa.glsl" ), "TAA", isDefault ) {
        m_interAttachments.resize( 1 ); // Prev Frame
    }

    void setup_out_attachments( std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies ) override;

    void setup_uniforms( std::vector<Graphics::Frame>& frames ) override;

    void create_framebuffer() override;

    void link_input_attachments() override;

    void execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0 ) override;
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif