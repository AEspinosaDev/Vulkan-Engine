/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FXAA_PASS_H
#define FXAA_PASS_H
#include <engine/core/renderpasses/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{
/*
Postprocess pass for antialasing final image using FXAA technique
*/
class FXAAPass : public RenderPass
{
    ColorFormatType m_colorFormat;
    Mesh *m_vignette;
    Graphics::DescriptorSet m_imageDescriptorSet;

  public:
    FXAAPass(Graphics::Context *ctx, Extent2D extent, uint32_t framebufferCount, ColorFormatType colorFormat,
             Mesh *vignette, bool isDefault = true)
        : RenderPass(ctx, extent, framebufferCount, 1, isDefault), m_colorFormat(colorFormat), m_vignette(vignette)
    {
    }

    void init();

    void create_descriptors();

    void create_graphic_pipelines();

    void render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void connect_to_previous_images(std::vector<Graphics::Image> images);
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif