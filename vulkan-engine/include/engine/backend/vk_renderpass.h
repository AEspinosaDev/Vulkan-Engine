#ifndef VK_RENDERPASS
#define VK_RENDERPASS

#include <engine/vk_common.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct RenderPassBuilder
{
    std::vector<VkAttachmentDescription> colorAttachments;
    VkAttachmentDescription depthAttachment{};

    void add_color_attachment(VkFormat &format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, bool stencil = true);
    void setup_depth_attachment(VkFormat &format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, bool stencil = true, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    VkRenderPass build_renderpass(VkDevice &device, bool colorBit, bool depthBit);
    VkRenderPass build_renderpass(VkDevice &device, bool colorBit, bool depthBit, std::vector<VkSubpassDependency> dependencies);
};

VULKAN_ENGINE_NAMESPACE_END

#endif