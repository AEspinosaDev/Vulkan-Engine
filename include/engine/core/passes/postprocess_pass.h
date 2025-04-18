/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef POSTPROCESS_PASS_H
#define POSTPROCESS_PASS_H
#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {
/*
Generic Postprocess Pass. It recieves an image from a previous pass and performs a postptocess task defined by a shader
on it. Can be inherited.
*/
class PostProcessPass : public GraphicPass
{
  protected:
    ColorFormatType         m_colorFormat;
    Graphics::DescriptorSet m_imageDescriptorSet;
    std::string             m_shaderPath;

  public:
    PostProcessPass(Graphics::Device* ctx,
                    Extent2D          extent,
                    ColorFormatType   colorFormat,
                    std::string       shaderPath,
                    std::string       name      = "POST-PROCESS",
                    bool              isDefault = true)
        : BasePass(ctx, extent, 1, 1, isDefault, name)
        , m_colorFormat(colorFormat)
        , m_shaderPath(shaderPath) {
    }

    virtual void setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                   std::vector<Graphics::SubPassDependency>& dependencies);

    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames);

    virtual void setup_shader_passes();

    virtual void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0);

    virtual void link_previous_images(std::vector<Graphics::Image> images);

    void clean_framebuffer() {
        GraphicPass::clean_framebuffer();
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif