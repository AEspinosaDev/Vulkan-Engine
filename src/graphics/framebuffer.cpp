#include <engine/graphics/framebuffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {
void Framebuffer::init(VulkanRenderPass&        renderpass,
                       Extent2D                 extent,
                       std::vector<Attachment>& attachments,
                       uint32_t                 layers) {

    m_device = renderpass.get_device_handle();
    m_layers = layers;

    std::vector<VkImageView> viewAttachments;
    viewAttachments.resize(attachments.size());

    // If default
    size_t presentViewIndex{0};

    for (size_t i = 0; i < viewAttachments.size(); i++)
    {
        viewAttachments[i] = attachments[i].image.view;
    }

    VkFramebufferCreateInfo fbInfo = Init::framebuffer_create_info(renderpass.get_handle(), extent);
    fbInfo.pAttachments            = viewAttachments.data();
    fbInfo.attachmentCount         = (uint32_t)viewAttachments.size();
    fbInfo.layers                  = layers;

    if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        throw VKFW_Exception("failed to create framebuffer!");
    }
}
void Framebuffer::cleanup() {
    if (m_handle != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(m_device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END