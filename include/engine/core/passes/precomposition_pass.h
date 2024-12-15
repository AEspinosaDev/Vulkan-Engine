/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PRECOMPOSITION_PASS_H
#define PRECOMPOSITION_PASS_H
#include <engine/core/passes/pass.h>
#include <engine/core/resource_manager.h>
#include <random>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {
    
typedef enum class AmbientOcclusionType
{
    SSAO = 0,
    RTAO = 1, // Raytraced AO
} AOType;
struct SSAOSettings {
    float    radius  = 0.75;
    float    bias    = 0.0;
    uint32_t samples = 16;
    AOType   type    = AOType::RTAO;
    uint32_t enabled = 1;
};
/*
Pre-composition pass called before the Composition (Lighting) pass in a deferred framework. This pass computes
SSAO and other lighting effects, such as blurring raytraced shadows by bilinear filering them, in order to be used in
future lighting passes.
*/
class PreCompositionPass : public GraphicPass
{
    Mesh* m_vignette;
    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;
    /*Resources*/
    const size_t     MAX_KERNEL_MEMBERS = 64;
    Graphics::Buffer m_kernelBuffer;
    bool             m_updateSamplesKernel = true;
    Graphics::Image  m_blurredSSAO;
    /*Settings*/
    SSAOSettings m_AO = {};

    void create_samples_kernel();

  public:
    PreCompositionPass(Graphics::Device* ctx, VkExtent2D extent, uint32_t framebufferCount, Mesh* vignette)
        : BasePass(ctx, extent, framebufferCount, 1, false, "PRE-COMPOSITION")
        , m_vignette(vignette) {
    }

    inline void set_SSAO_settings(SSAOSettings settings) {
        if (settings.samples != m_AO.samples)
            m_updateSamplesKernel = true;
        m_AO = settings;
    };
    inline SSAOSettings get_SSAO_settings() const {
        return m_AO;
    };

    void setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void connect_to_previous_images(std::vector<Graphics::Image> images);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void cleanup();
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif