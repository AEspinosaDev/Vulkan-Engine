/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SSAO_BLUR_PASS_H
#define SSAO_BLUR_PASS_H

#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class SSAOBlurPass : public RenderPass
{
    Mesh *m_vignette;

    DescriptorManager m_descriptorManager{};

    DescriptorSet m_descriptorSet{};

    Image m_ssao;

public:
    SSAOBlurPass(Context* ctx, VkExtent2D extent,
                 uint32_t framebufferCount,
                 Mesh *vignette) : RenderPass(ctx, extent, framebufferCount),
                                   m_vignette(vignette) {}

    void init();

    void create_descriptors(uint32_t framesPerFlight);

    void create_pipelines(DescriptorManager &descriptorManager);

    void init_resources();

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void cleanup();

    inline void set_ssao_buffer(Image ssao)
    {
        m_ssao = ssao;
    }

    void update();
};

VULKAN_ENGINE_NAMESPACE_END

#endif
