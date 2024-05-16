/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMPOSITION_PASS_H
#define COMPOSITION_PASS_H
#include <engine/core/renderpass.h>

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
    CompositionPass(VkExtent2D extent,
                    uint32_t framebufferCount,
                    ColorFormatType colorFormat, Mesh *vignette, bool fxaa) : RenderPass(extent, framebufferCount, 1, fxaa ? false : true),
                                                                   m_colorFormat(colorFormat), m_vignette(vignette), m_fxaa(fxaa) {}

    void init(VkDevice &device);

    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void init_resources(VkDevice &device,
                        VkPhysicalDevice &gpu,
                        VmaAllocator &memory,
                        VkQueue &gfxQueue,
                        utils::UploadContext &uploadContext);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    inline void set_output_type(int op) { m_outputType = op; }
    inline int get_output_type() const { return m_outputType; }
    
    void set_g_buffer(Image position, Image normals, Image albedo, Image material, DescriptorManager &descriptorManager);

    void update_uniforms(VmaAllocator &memory);

    void cleanup(VkDevice &device, VmaAllocator &memory);

     void update(VkDevice &device, VmaAllocator &memory, Swapchain *swp = nullptr);
};
VULKAN_ENGINE_NAMESPACE_END

#endif