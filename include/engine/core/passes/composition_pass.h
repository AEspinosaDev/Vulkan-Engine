/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMPOSITION_PASS_H
#define COMPOSITION_PASS_H
#include <engine/core/passes/pass.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {

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

class CompositionPass : public GraphicPass
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    Mesh*           m_vignette;

    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet gBufferDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    Graphics::Image m_prevFrame;

    struct Settings {
        OutputBuffer outputBuffer = OutputBuffer::LIGHTING;
        int          enableAO     = 1;
        int          AOtype       = 0;
        VXGI         vxgi         = {};
        SSR          ssr          = {};
    };
    Settings m_settings = {};

    void create_prev_frame_image();

  public:
    CompositionPass(Graphics::Device* ctx,
                    VkExtent2D        extent,
                    ColorFormatType   colorFormat,
                    Mesh*             vignette,
                    bool              isDefault = true)
        : BasePass(ctx, extent, 1, 1, isDefault, "COMPOSITION")
        , m_colorFormat(colorFormat)
        , m_vignette(vignette) {
    }

    inline void set_SSR_settings(SSR settings) {
        m_settings.ssr = settings;
    };
    inline SSR get_SSR_settings() const {
        return m_settings.ssr;
    };
    inline void set_VXGI_settings(VXGI settings) {
        m_settings.vxgi = settings;
    };
    inline VXGI get_VXGI_settings() const {
        return m_settings.vxgi;
    };
    inline void set_output_buffer(OutputBuffer buffer) {
        m_settings.outputBuffer = buffer;
    };
    inline OutputBuffer get_output_buffer() const {
        return m_settings.outputBuffer;
    };
    inline bool enable_AO() const {
        return m_settings.enableAO;
    }
    inline void enable_AO(bool op) {
        m_settings.enableAO = op;
    }
    inline int get_AO_type() const {
        return m_settings.AOtype;
    }
    inline void set_AO_type(int op) {
        m_settings.AOtype = op;
    }

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void link_previous_images(std::vector<Graphics::Image> images);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void set_envmap_descriptor(Graphics::Image env, Graphics::Image irr);

    void set_voxelization_descriptor(Graphics::Image voxel);

    void update();

    void cleanup();
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif