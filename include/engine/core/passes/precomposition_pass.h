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
    VXAO = 2, // Voxel Cone Traced
} AOType;
struct AO {
    float    radius     = 0.2;
    float    bias       = 0.0;
    uint32_t samples    = 4;
    AOType   type       = AOType::SSAO;
    uint32_t enabled    = 1;
    float    blurRadius = 4.0f;
    float    blurSigmaA = 10.0f;
    float    blurSigmaB = 0.8f;
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
        Graphics::DescriptorSet blurImageDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;
    /*Resources*/
    const size_t     MAX_KERNEL_MEMBERS = 64;
    Graphics::Buffer m_kernelBuffer;
    bool             m_updateSamplesKernel = true;
    /*Settings*/
    AO m_AO = {};

    void create_samples_kernel();

  public:
    PreCompositionPass(Graphics::Device* ctx, VkExtent2D extent, Mesh* vignette)
        : BasePass(ctx, extent, 2, 1, false, "PRE-COMPOSITION")
        , m_vignette(vignette) {
    }

    inline void set_SSAO_settings(AO settings) {
        if (settings.samples != m_AO.samples)
            m_updateSamplesKernel = true;
        m_AO = settings;
    };
    inline AO get_SSAO_settings() const {
        return m_AO;
    };

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void link_previous_images(std::vector<Graphics::Image> images);

    void update_uniforms(uint32_t frameIndex, Scene* const scene);

    void cleanup();
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif