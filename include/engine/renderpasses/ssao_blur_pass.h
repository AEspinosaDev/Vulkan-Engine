#ifndef SSAO_BLUR_PASS_H
#define SSAO_BLUR_PASS_H
#include <random>

#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class SSAOBlurPass : public RenderPass
{
    Mesh *m_vignette;

    DescriptorManager m_descriptorManager{};

    DescriptorSet m_descriptorSet{};

    Image m_ssao;

public:
    SSAOBlurPass(VkExtent2D extent,
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

    inline void set_ssao_buffer(Image ssao)
    {
       m_ssao = ssao;
    }

      void update(VkDevice &device, VmaAllocator &memory, Swapchain *swp = nullptr);
};

VULKAN_ENGINE_NAMESPACE_END

#endif
