// #ifndef GEOMETRY_PASS_H
// #define GEOMETRY_PASS_H
// #include <engine/core/renderpass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN

// class GeometryPass : public RenderPass
// {
//     ColorFormatType m_colorFormat;
//     DepthFormatType m_depthFormat;
//     VkSampleCountFlagBits m_samples;

// public:
//     GeometryPass(VkExtent2D extent,
//                 uint32_t framebufferCount,
//                 ColorFormatType colorFormat,
//                 DepthFormatType depthFormat,
//                 VkSampleCountFlagBits samples) : RenderPass(extent, framebufferCount, 1, true),
//                                                  m_colorFormat(colorFormat),
//                                                  m_depthFormat(depthFormat),
//                                                  m_samples(samples) {}
//     void init(VkDevice &device);

//     void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

//     void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

// };
// VULKAN_ENGINE_NAMESPACE_END

// #endif