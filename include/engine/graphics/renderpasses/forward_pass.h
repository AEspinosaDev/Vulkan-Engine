/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FORWARD_PASS_H
#define FORWARD_PASS_H
#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class ForwardPass : public RenderPass
{
    ColorFormatType m_colorFormat;
    DepthFormatType m_depthFormat;
    AntialiasingType m_aa;

public:
    ForwardPass(Context *ctx,
                VkExtent2D extent,
                uint32_t framebufferCount,
                ColorFormatType colorFormat,
                DepthFormatType depthFormat,
                AntialiasingType samples) : RenderPass(ctx, extent, framebufferCount, 1, samples == AntialiasingType::FXAA ? false : true),
                                            m_colorFormat(colorFormat),
                                            m_depthFormat(depthFormat),
                                            m_aa(samples) {}
    void init();

    void create_pipelines(DescriptorManager &descriptorManager);

    void init_resources();

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void update();
};
VULKAN_ENGINE_NAMESPACE_END

#endif