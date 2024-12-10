/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SSR_PASS_H
#define SSR_PASS_H
#include <engine/core/passes/postprocess_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {
/*
Raymarched depth-based Screen Space Reflections pass.
*/
class SSRPass : public PostProcessPass
{
  protected:
    /*Descriptors*/
    struct FrameDescriptors {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet gBufferDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    uint32_t m_steps = 20;

  public:
    SSRPass(Graphics::Device* ctx,
            Extent2D          extent,
            uint32_t          framebufferCount,
            ColorFormatType   colorFormat,
            Mesh*             vignette,
            bool              isDefault = false)
        : PostProcessPass(ctx,
                          extent,
                          framebufferCount,
                          colorFormat,
                          vignette,
                          ENGINE_RESOURCES_PATH "shaders/misc/SSR.glsl",
                          "SSR",
                          isDefault) {
    }

    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames);

    virtual void setup_shader_passes();

    virtual void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    virtual void connect_to_previous_images(std::vector<Graphics::Image> images);
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif