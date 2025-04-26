/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BLOOM_PASS_H
#define BLOOM_PASS_H

#include <engine/core/passes/graphic_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
Physically Based Bloom Pass.

The method that inspired this article was presented at ACM Siggraph in 2014 by Jorge Jimenez for Call of Duty: Advanced
Warfare.

The algorithm follows this recipe:
- Run a shader which downsamples (downscales) the HDR buffer containing per-pixel color with light and shadows applied.
This shader is run a fixed number of times to continually produce a smaller image, each time with half resolution in
both X and Y axes.
- We then run a small 3x3 filter kernel on each downsampled image, and progressively upsample them until we reach image
A (first downsampled image).
- Finally, we mix the overall bloom contribution into the HDR source image, with a strong bias towards the HDR source.
*/
class BloomPass : public BaseGraphicPass
{
  protected:
    ColorFormatType m_colorFormat = SRGBA_16F;

    Graphics::DescriptorSet m_imageDescriptorSet;

    const uint32_t MIPMAP_LEVELS = 6;

    // Settings
    float m_bloomStrength = 0.05f;

    // Resources
    Graphics::Image                    m_originalImage;
    Graphics::Image                    m_brightImage;
    Graphics::Image                    m_bloomImage;
    std::vector<Graphics::Image>       m_bloomMipmaps;
    std::vector<Graphics::Framebuffer> m_bloomFramebuffers;

  public:
    /*
          Input Attachments:
          -
          - Lighting
          - Bright Lighting (HDR)

          Output Attachments:
          -
          - Bloom + Lighting

      */
    BloomPass(Graphics::Device* device, const PassLinkage<2, 1>& config, Extent2D extent, bool isDefault = false)
        : BaseGraphicPass(device, extent, 1, 1, true, isDefault, "BLOOM") {
        BasePass::store_attachments<2, 1>(config);
    }

    inline float get_bloom_strength() const {
        return m_bloomStrength;
    }
    inline void set_bloom_strength(float st) {
        m_bloomStrength = st;
    }

    void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                               std::vector<Graphics::SubPassDependency>& dependencies) override;

    void setup_uniforms(std::vector<Graphics::Frame>& frames) override;

    void setup_shader_passes() override;

    void execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0) override;

    void link_input_attachments() override;

    void resize_attachments() override;

    void cleanup() override;
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif