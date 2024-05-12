#ifndef GEOMETRY_PASS_H
#define GEOMETRY_PASS_H
#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class GeometryPass : public RenderPass
{
    DepthFormatType m_depthFormat;

public:
    GeometryPass(VkExtent2D extent,
                 uint32_t framebufferCount,
                 DepthFormatType depthFormat ) : RenderPass(extent, 1, 1),
                                                  m_depthFormat(depthFormat) {}
    void init(VkDevice &device);

    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);
};
VULKAN_ENGINE_NAMESPACE_END

#endif