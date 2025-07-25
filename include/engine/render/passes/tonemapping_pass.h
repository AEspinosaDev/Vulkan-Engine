/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TONEMAPPING_PASS_H
#define TONEMAPPING_PASS_H
#include <engine/render/passes/postprocess_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

using namespace Core;
using namespace Graphics;

namespace Render {

enum TonemappingType
{
    FILMIC_TONEMAP    = 0,
    REINDHART_TONEMAP = 1,
    ACES_TONEMAP      = 2,
    NO_TONEMAP        = 3,
};

/*
Tonemapping Pass.
*/
class TonemappingPass final : public PostProcessPass<1, 1>
{
protected:
    float           m_exposure = 1.0f;
    TonemappingType m_tonemap  = FILMIC_TONEMAP;

    void setup_input_image_mipmaps();

public:
    /*

               Input Attachments:
               -
               - Color

               Output Attachments:
               -
               - Tonemapped Color

           */
    TonemappingPass( const ptr<Graphics::Device>& device, const ptr<Render::GPUResourcePool>& shared, const PassLinkage<1, 1>& config, Extent2D extent, ColorFormatType colorFormat, bool isDefault = true )
        : PostProcessPass( device, shared, config, extent, colorFormat, GET_RESOURCE_PATH( "shaders/misc/tonemapping.glsl" ), "TONEMAPPING", isDefault ) {
        m_interAttachments.resize( 1 );
    }

    inline float get_exposure() const {
        return m_exposure;
    }
    inline void set_exposure( float exposure ) {
        m_exposure = exposure;
    }
    inline TonemappingType get_tonemapping_type() const {
        return m_tonemap;
    }
    inline void set_tonemapping_type( TonemappingType type ) {
        m_tonemap = type;
    }

    void setup_shader_passes() override;

    void execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0 ) override;
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif