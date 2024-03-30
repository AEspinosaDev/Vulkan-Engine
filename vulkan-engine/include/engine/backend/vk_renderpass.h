/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VK_RENDERPASS
#define VK_RENDERPASS

#include <engine/vk_common.h>
#include <engine/vk_texture.h>

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

    std::vector<VkFramebuffer> framebuffers;
    std::vector<AttachmentDescription> attachmentsInfo;

    std::vector<Texture *> textureAttachments;

    static void begin(VkCommandBuffer &cmd, RenderPass &pass, VkExtent2D extent,
               std::vector<VkClearValue> clearValues,
               uint32_t framebufferId = 0, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
    static void end(VkCommandBuffer &cmd);
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