/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FORWARD_PASS_H
#define FORWARD_PASS_H
#include <engine/core/renderpasses/renderpass.h>
#include <engine/core/textures/texture.h>
#include <engine/core/textures/textureLDR.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

class ForwardPass : public RenderPass
{
    /*Setup*/
    ColorFormatType m_colorFormat;
    DepthFormatType m_depthFormat;
    MSAASamples m_aa;

    /*Descriptors*/
    struct FrameDescriptors
    {
        Graphics::DescriptorSet globalDescritor;
        Graphics::DescriptorSet objectDescritor;
    };
    std::vector<FrameDescriptors> m_descriptors;

    void setup_material_descriptor(IMaterial *mat);

  public:
    ForwardPass(Graphics::Context *ctx, Extent2D extent, uint32_t framebufferCount, ColorFormatType colorFormat,
                DepthFormatType depthFormat, MSAASamples samples, bool isDefault = true)
        : RenderPass(ctx, extent, framebufferCount, 1, isDefault), m_colorFormat(colorFormat),
          m_depthFormat(depthFormat), m_aa(samples)
    {
    }

    void init();

    void create_descriptors();

    void create_graphic_pipelines();

    void render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void upload_data(uint32_t frameIndex, Scene *const scene);

    void connect_to_previous_images(std::vector<Graphics::Image> images);

    void set_envmap_descriptor(Graphics::Image env, Graphics::Image irr);
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif