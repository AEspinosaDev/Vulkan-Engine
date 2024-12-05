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

class BloomPass : public BasePass
{
  protected:
    ColorFormatType m_colorFormat = SRGBA_32F;
    Mesh*           m_vignette;

    Graphics::DescriptorSet m_imageDescriptorSet;

    const uint32_t               MIPMAP_LEVELS = 6;
    Graphics::Image              m_originalImage;
    Graphics::Image              m_brightImage;

    Graphics::Image              m_bloomImage;
    std::vector<Graphics::Image> m_bloomMipmaps;

  public:
    BloomPass(Graphics::Device* ctx, Extent2D extent, uint32_t framebufferCount, Mesh* vignette, bool isDefault = false)
        : BasePass(ctx, extent, framebufferCount, 1, isDefault, "BLOOM")
        , m_vignette(vignette) {
    }

    void setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                           std::vector<Graphics::SubPassDependency>& dependencies);

    void setup_uniforms(std::vector<Graphics::Frame>& frames);

    void setup_shader_passes();

    void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    void connect_to_previous_images(std::vector<Graphics::Image> images);

    void update();
    
    void cleanup();


};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif