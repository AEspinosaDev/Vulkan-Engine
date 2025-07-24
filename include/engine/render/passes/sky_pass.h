/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SKY_PASS_H
#define SKY_PASS_H
#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

using namespace Core;
using namespace Graphics;

namespace Render {
/*
Procedural Physically Based Sky generation pass. Based on "A Scalable and Production Ready Sky and Atmosphere Rendering
Technique" by Sébastien Hillaire (2020).

Transmittance LUT is based on "Precomputed Atmospheric Scattering" by Eric Bruneton and Fabrice Neyret (2008).

Implementation by Fernando García Liñán 2023
*/
class SkyPass final : public BaseGraphicPass
{
protected:
    Graphics::DescriptorSet m_imageDescriptor;

    /*
     * Every aerosol type expects 5 parameters:
     * - Scattering cross section
     * - Absorption cross section
     * - Base density (km^-3)
     * - Background density (km^-3)
     * - Height scaling parameter
     * These parameters can be sent as uniforms.
     *
     * This model for aerosols and their corresponding parameters come from
     * "A Physically-Based Spatio-Temporal Sky Model"
     * by Guimera et al. (2018).
     */
    struct AerosolParams {
        Vec4  aerosolAbsorptionCrossSection = Vec4( 0.0 );
        Vec4  aerosolScatteringCrossSection = Vec4( 0.0 );
        float aerosolBaseDensity            = 0.0f;
        float aerosolBackgroundDensity      = 0.0f;
        float aerosolHeightScale            = 0.0f;
    };

    AerosolParams get_aerosol_params( AerosolType type );

public:
    /*
          Output Attachments:
          -
          - Sky Cubemap
      */
    SkyPass( const ptr<Graphics::Device>& device, const ptr<Render::GPUResourcePool>& shared, const PassLinkage<0, 1>& config, Extent2D extent )
        : BaseGraphicPass( device, shared, extent, 3, 1, false, false, "SKY GENERATION" ) {
        BasePass::store_attachments<0, 1>( config );
    }

    void create_framebuffer() override;

    void setup_out_attachments( std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies ) override;

    void setup_uniforms( std::vector<Graphics::Frame>& frames ) override;

    void setup_shader_passes() override;

    void resize_attachments() override;

    void execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0 ) override;
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif