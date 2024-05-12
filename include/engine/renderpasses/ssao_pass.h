#ifndef SSAO_PASS_H
#define SSAO_PASS_H
#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class SSAOPass : public RenderPass
{
    Mesh *m_vignette;

    DescriptorManager m_descriptorManager;

    // Texture descriptor only
    // rest is not necessary

public:
    SSAOPass(VkExtent2D extent,
             Mesh *vignette) : RenderPass(extent, 1),
                               m_vignette(vignette) {}

    void init(VkDevice &device);

    void create_descriptors(VkDevice &device, VkPhysicalDevice &gpy, VmaAllocator &memory, uint32_t framesPerFlight);

    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);
};

VULKAN_ENGINE_NAMESPACE_END

#endif
