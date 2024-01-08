#ifndef VK_RENDERPASS
#define VK_RENDERPASS

#include "vk_core.h"

namespace vke
{

    struct RenderPassBuilder
    {
        std::vector<VkAttachmentDescription> colorAttachments;
        VkAttachmentDescription depthAttachment{};

        void add_color_attachment(VkFormat& format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, bool stencil = true);
        void setup_depth_attachment(VkFormat& format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, bool stencil = true);

        VkRenderPass build_renderpass(VkDevice& device, bool colorBit, bool depthBit);
        VkRenderPass build_renderpass(VkDevice& device, bool colorBit, bool depthBit, std::vector<VkSubpassDependency> dependencies);
    };

}

#endif