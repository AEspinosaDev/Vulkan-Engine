/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PAN_CONV_PASS_H
#define PAN_CONV_PASS_H
#include <engine/core/renderpasses/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

class PanroramaConverterPass : public RenderPass
{
    Graphics::DescriptorSet m_panoramaDescriptorSet;

    Mesh *m_vignette;

  public:
    PanroramaConverterPass(Graphics::Context *ctx, Extent2D extent, Mesh *vignette)
        : RenderPass(ctx, extent, 1, 6, false), m_vignette(vignette)
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