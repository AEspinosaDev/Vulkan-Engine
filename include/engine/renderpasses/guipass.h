#ifndef GUI_PASS_H
#define GUI_PASS_H
#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class GUIPass : public RenderPass
{
    VkSampleCountFlagBits m_samples;
    ColorFormatType m_colorFormat;
    DepthFormatType m_depthFormat;
    GUIOverlay *m_gui;

public:
    GUIPass(VkExtent2D extent,
                uint32_t framebufferCount,
                ColorFormatType colorFormat,
                DepthFormatType depthFormat,
                VkSampleCountFlagBits samples, GUIOverlay *gui) : RenderPass(extent, framebufferCount, 1, true),
                                                 m_colorFormat(colorFormat), m_depthFormat(depthFormat), m_samples(samples), m_gui(gui) {}
    void init(VkDevice &device);
    
    void init_shaderpasses(VkDevice &device, DescriptorManager &descriptorManager);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t framebufferIndex = 0);

};
VULKAN_ENGINE_NAMESPACE_END
#endif