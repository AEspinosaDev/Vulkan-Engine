#ifndef COMPOSITION_PASS_H
#define COMPOSITION_PASS_H
#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class CompositionPass : public RenderPass
{

    ColorFormatType m_colorFormat;

    Mesh *m_vignette;
    GUIOverlay *m_gui;

    DescriptorSet m_GBufferDescriptor{};

    std::vector<Image> m_Gbuffer;
    Buffer m_uniformBuffer;

    unsigned int m_outputType{0};

public:
    CompositionPass(VkExtent2D extent,
                    uint32_t framebufferCount,
                    ColorFormatType colorFormat, Mesh *vignette) : RenderPass(extent, framebufferCount, 1, true),
                                                                   m_colorFormat(colorFormat), m_vignette(vignette) {}

    void init(VkDevice &device);

    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void init_resources(VkDevice &device,
                        VkPhysicalDevice &gpu,
                        VmaAllocator &memory,
                        VkQueue &gfxQueue,
                        utils::UploadContext &uploadContext);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    inline void set_gui(GUIOverlay *gui) { m_gui = gui; }

    inline void set_output_type(int op) { m_outputType = op; }
    inline int get_output_type() const { return m_outputType; }
    
    void set_g_buffer(Image position, Image normals, Image albedo, Image material, DescriptorManager &descriptorManager);

    void update_aux_uniforms(VmaAllocator &memory);

    void cleanup(VkDevice &device, VmaAllocator &memory);
};
VULKAN_ENGINE_NAMESPACE_END

#endif