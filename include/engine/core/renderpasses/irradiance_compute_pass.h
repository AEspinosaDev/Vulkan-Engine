/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef IRR_COMP_PASS_H
#define IRR_COMP_PASS_H
#include <engine/core/renderpasses/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

class IrrandianceComputePass : public RenderPass
{
    ColorFormatType m_format;
    Graphics::DescriptorSet m_captureDescriptorSet;
    Graphics::Buffer m_captureBuffer;

  public:
    IrrandianceComputePass(Graphics::Context *ctx, ColorFormatType format, Extent2D extent)
        : RenderPass(ctx, extent, 1, CUBEMAP_FACES, false),  m_format(format)
    {
    }

    void init();

    void create_descriptors();

    void create_graphic_pipelines();

    void render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void upload_data(uint32_t frameIndex, Scene *const scene);

    void connect_env_cubemap(Graphics::Image env);

    void cleanup();
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif