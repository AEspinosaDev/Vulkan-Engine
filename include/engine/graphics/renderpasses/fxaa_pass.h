/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FXAA_PASS_H
#define FXAA_PASS_H
#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class FXAAPass : public RenderPass
{
    ColorFormatType m_colorFormat;
    Mesh *m_vignette;

    DescriptorManager m_descriptorManager{};
    DescriptorSet m_descriptorSet;

    Image m_outputBuffer;

public:
    FXAAPass(Context* ctx, VkExtent2D extent,
             uint32_t framebufferCount,
             ColorFormatType colorFormat, Mesh *vignette) : RenderPass(ctx, extent, framebufferCount, 1, true),
                                                            m_colorFormat(colorFormat), m_vignette(vignette) {}

    void init();

    void create_descriptors(uint32_t framesPerFlight);

    void create_pipelines(DescriptorManager &descriptorManager);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void set_output_buffer(Image output);

    void cleanup();

};
VULKAN_ENGINE_NAMESPACE_END

#endif