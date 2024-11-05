/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PAN_CONV_PASS_H
#define PAN_CONV_PASS_H
#include <engine/core/renderpasses/renderpass.h>
#include <engine/core/textures/textureHDR.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

class PanoramaConverterPass : public RenderPass
{
    ColorFormatType m_format;
    Graphics::DescriptorSet m_panoramaDescriptorSet;
    Mesh *m_vignette;

  public:
    PanoramaConverterPass(Graphics::Device *ctx, ColorFormatType format, Extent2D extent, Mesh *vignette)
        : RenderPass(ctx, extent, 1, CUBEMAP_FACES, false), m_vignette(vignette), m_format(format)
    {
    }

    void init();

    void create_descriptors();

    void create_graphic_pipelines();

    void render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void upload_data(uint32_t frameIndex, Scene *const scene);

    void connect_to_previous_images(std::vector<Graphics::Image> images);
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif