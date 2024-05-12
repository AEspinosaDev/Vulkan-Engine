#ifndef SSAO_PASS_H
#define SSAO_PASS_H
#include <random>

#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class SSAOPass : public RenderPass
{
    Mesh *m_vignette;

    const size_t KERNEL_MEMBERS = 64;

    DescriptorManager m_descriptorManager;

    DescriptorSet m_descriptorSet;
    Buffer m_kernelBuffer;

public:
    SSAOPass(VkExtent2D extent,
             Mesh *vignette) : RenderPass(extent, 1),
                               m_vignette(vignette) {}

    void init(VkDevice &device);

    void create_descriptors(VkDevice &device, VkPhysicalDevice &gpu, VmaAllocator &memory, uint32_t framesPerFlight);

    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void init_resources(VkDevice &device,
                        VkPhysicalDevice &gpu,
                        VmaAllocator &memory,
                        VkQueue &gfxQueue,
                        utils::UploadContext &uploadContext);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void cleanup(VkDevice &device, VmaAllocator &memory);
};

VULKAN_ENGINE_NAMESPACE_END

#endif
