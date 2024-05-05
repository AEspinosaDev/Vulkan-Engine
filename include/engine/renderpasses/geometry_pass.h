// #ifndef DEPTH_PASS_H
// #define DEPTH_PASS_H
// #include <engine/core/renderpass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN

// class DepthPass : public RenderPass
// {
//     VkSampleCountFlagBits m_samples;
//     DepthFormatType m_depthFormat;

// public:
//     DepthPass(VkExtent2D extent,
//               uint32_t framebufferCount,
//               DepthFormatType depthFormat,
//               VkSampleCountFlagBits samples) : RenderPass(extent, framebufferCount, 1, false),
//                                                m_depthFormat(depthFormat), m_samples(samples) {}

//     void init(VkDevice &device);

//     void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

//     void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t framebufferIndex = 0);
// };
// VULKAN_ENGINE_NAMESPACE_END

// #endif