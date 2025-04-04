/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BLOOM_PASS_H
#define BLOOM_PASS_H

#include <engine/core/passes/pass.h>

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
class BloomPass : public BasePass
{
  protected:
    ColorFormatType m_colorFormat = SRGBA_32F;

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
    BloomPass(Graphics::Device* ctx, Extent2D extent, bool isDefault = false)
        : BasePass(ctx, extent, 1, 1, isDefault, "BLOOM") {
    }

    inline float get_bloom_strength() const {
        return m_bloomStrength;
    }
    inline void set_bloom_strength(float st) {
        m_bloomStrength = st;
    }

    void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void link_previous_images(std::vector<Graphics::Image> images);

    void update_framebuffer();

    void cleanup();
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif