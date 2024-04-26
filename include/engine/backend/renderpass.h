/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <engine/common.h>
#include <engine/backend/swapchain.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct AttachmentDescription
{
    VkAttachmentDescription description{};
    VkImageUsageFlags viewUsage;
    VkImageAspectFlags viewAspect;
    VkImageViewType viewType;

    AttachmentDescription(VkAttachmentDescription d) : description(d){};
    AttachmentDescription(VkAttachmentDescription d,
                          VkImageUsageFlags usage,
                          VkImageAspectFlags aspect,
                          VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D) : description(d), viewUsage(usage), viewAspect(aspect), viewType(type){};
};
struct RenderPass
{
    VkExtent2D extent;

    VkRenderPass obj;

    uint32_t framebufferCount;
    uint32_t attachmentCount;

    std::vector<VkFramebuffer> framebuffers;
    std::vector<AttachmentDescription> attachmentsInfo;
    std::vector<Image> imageAttachments;

    bool isFramebufferRecreatable{true};

    /**
     * Begin render pass
     */
    static void begin(VkCommandBuffer &cmd, RenderPass &pass, VkExtent2D extent,
                      std::vector<VkClearValue> clearValues,
                      uint32_t framebufferId = 0, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
    /**
     * End render pass
     */
    static void end(VkCommandBuffer &cmd);

    /**
     * Create framebuffers and images attached to them necessary for the renderpass to work
     */
    void create_framebuffer(VkDevice &device, VmaAllocator &memory, VkExtent2D extent, uint32_t layers = 1, uint32_t count = 1, Swapchain *swp = nullptr);
    /**
     * Destroy framebuffers and images attached to them necessary for the renderpass to work. If images have a sampler attached to them, contol the destruction of it too.
     */
    void clean_framebuffer(VkDevice &device, VmaAllocator &memory, bool destroyImageSamplers = true);
    /**
     * Destroy the renderpass object only.
     */
    void cleanup(VkDevice &device);
};

struct RenderPassBuilder
{

    std::vector<AttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpasses;
    std::vector<VkSubpassDependency> dependencies;

    inline void add_attachment(AttachmentDescription attachment)
    {
        attachments.push_back(attachment);
    };
    inline void add_subpass(VkSubpassDescription subpass)
    {
        subpasses.push_back(subpass);
    };
    inline void add_dependency(VkSubpassDependency dep)
    {
        dependencies.push_back(dep);
    };

    inline void clear_cache()
    {
        attachments.clear();
        subpasses.clear();
        dependencies.clear();
    };

    RenderPass build_renderpass(VkDevice &device);
};

VULKAN_ENGINE_NAMESPACE_END

#endif