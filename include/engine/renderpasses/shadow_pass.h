#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H
#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class ShadowPass : public RenderPass
{
    DepthFormatType m_depthFormat;

public:
    ShadowPass(VkExtent2D extent,
               uint32_t numLights,
               DepthFormatType depthFormat) : RenderPass(extent, 1, numLights),
                                              m_depthFormat(depthFormat) {}

    void init(VkDevice &device);
    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void init_resources(VkDevice &device,
                        VkPhysicalDevice &gpu,
                        VmaAllocator &memory,
                        VkQueue &gfxQueue,
                        utils::UploadContext &uploadContext);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);
};

VULKAN_ENGINE_NAMESPACE_END

#endif
