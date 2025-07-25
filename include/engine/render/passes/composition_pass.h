/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMPOSITION_PASS_H
#define COMPOSITION_PASS_H
#include <engine/render/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Core;
using namespace Graphics;

namespace Render {

/*
DEFERRED RENDERING LIGHTING PASS
*/
enum class OutputBuffer
{
    LIGHTING = 0,
    ALBEDO   = 1,
    NORMAL   = 2,
    POSITION = 3,
    MATERIAL = 4,
    SSAO     = 5,
    EMISSIVE = 6,
    SSR      = 7,
};
struct SSR { // Settings for Screen Space Reflections
    uint32_t maxSteps              = 64;
    float    stride                = 0.1f;
    uint32_t binaryRefinementSteps = 6;
    float    thickness             = 0.2f;
    float    jitter                = 0.1f;
    uint32_t enabled               = 1;
};
struct VXGI { // Settings for Voxel Based GI
    float    strength          = 1.0f;
    float    diffuseConeSpread = 0.577f;
    float    offset            = 1.0f;
    float    maxDistance       = 35.0f;
    uint32_t resolution        = 256;
    uint32_t samples           = 8;
    uint32_t enabled           = 1;
    uint32_t updateMode        = 0;
};

class CompositionPass final : public BaseGraphicPass
{
    /*Setup*/
    ColorFormatType m_colorFormat;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet gBufferDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    struct Settings {
        OutputBuffer outputBuffer = OutputBuffer::LIGHTING;
        int          enableAO     = 1;
        int          AOtype       = 0;
        VXGI         vxgi         = {};
        SSR          ssr          = {};
    };
    Settings m_settings = {};

public:
    /*
            Input Attachments:
            -
            - Lighting
            - Bright Lighting (HDR)

            Output Attachments:
            - Lighting
            - Bright Lighting (HDR)

        */
    CompositionPass( const ptr<Graphics::Device>&     device,
                     const ptr<Render::GPUResourcePool>& shared,
                     const PassLinkage<10, 2>&        config,
                     VkExtent2D                       extent,
                     ColorFormatType                  colorFormat,
                     bool                             isDefault = false )
        : BaseGraphicPass( device, shared, extent, 1, 1, true, isDefault, "COMPOSITION" )
        , m_colorFormat( colorFormat ) {
        BasePass::store_attachments<10, 2>( config );
    }

    inline void set_SSR_settings( SSR settings ) {
        m_settings.ssr = settings;
    };
    inline SSR get_SSR_settings() const {
        return m_settings.ssr;
    };
    inline void set_VXGI_settings( VXGI settings ) {
        m_settings.vxgi = settings;
    };
    inline VXGI get_VXGI_settings() const {
        return m_settings.vxgi;
    };
    inline void set_output_buffer( OutputBuffer buffer ) {
        m_settings.outputBuffer = buffer;
    };
    inline OutputBuffer get_output_buffer() const {
        return m_settings.outputBuffer;
    };
    inline bool enable_AO() const {
        return m_settings.enableAO;
    }
    inline void enable_AO( bool op ) {
        m_settings.enableAO = op;
    }
    inline int get_AO_type() const {
        return m_settings.AOtype;
    }
    inline void set_AO_type( int op ) {
        m_settings.AOtype = op;
    }

    void setup_out_attachments( std::vector<Graphics::AttachmentConfig>&  attachments,
                                std::vector<Graphics::SubPassDependency>& dependencies ) override;

    void setup_uniforms( std::vector<Graphics::Frame>& frames ) override;

    void setup_shader_passes() override;

    void execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0 ) override;

    void link_input_attachments() override;

    void update_uniforms( uint32_t frameIndex, Scene* const scene ) override;
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif