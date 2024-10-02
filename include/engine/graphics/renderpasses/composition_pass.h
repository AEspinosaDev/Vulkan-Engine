/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMPOSITION_PASS_H
#define COMPOSITION_PASS_H
#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class CompositionPass : public RenderPass
{

    ColorFormatType m_colorFormat;

    Mesh *m_vignette;

    bool m_fxaa;

    DescriptorSet m_GBufferDescriptor{};

    std::vector<Image> m_Gbuffer;
    Buffer m_uniformBuffer;

    unsigned int m_outputType{0};

public:
    CompositionPass(Context* ctx,VkExtent2D extent,
                    uint32_t framebufferCount,
                    ColorFormatType colorFormat, Mesh *vignette, bool fxaa) : RenderPass(ctx, extent, framebufferCount, 1, fxaa ? false : true),
                                                                   m_colorFormat(colorFormat), m_vignette(vignette), m_fxaa(fxaa) {}

    void init();

    void create_pipelines( DescriptorManager &descriptorManager);

    void init_resources();

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    inline void set_output_type(int op) { m_outputType = op; }
    inline int get_output_type() const { return m_outputType; }
    
    void set_g_buffer(Image position, Image normals, Image albedo, Image material, DescriptorManager &descriptorManager);

    void update_uniforms();

    void cleanup();

     void update();
};
VULKAN_ENGINE_NAMESPACE_END

#endif