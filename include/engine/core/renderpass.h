/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <engine/common.h>

#include <engine/backend/swapchain.h>
#include <engine/backend/frame.h>
#include <engine/backend/descriptors.h>

#include <engine/scene/scene.h>

#include <engine/utilities/gui.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

/*
Description info needed for CREATING a attachment that will be set in a renderpass and a its framebuffer
*/
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

/*
Attachment infor needed for USING a renderpasses and its framebuffers.
*/
struct Attachment
{
    Image image{};
    AttachmentDescription description;
    VkClearValue clearValue{};

    Attachment(AttachmentDescription descr,
               VkClearValue clearVal = {{{0.0, 0.0, 0.0, 1.0}}}) : description(descr),
                                                                   clearValue(clearVal) { clearValue.depthStencil.depth = 1.0f; };
};

/*
Core base class needed for rendering.
It controls the flow of the render state, what information and how it is being rendered.
It also gives access to the framebuffers containing the rendered data.
It can be inherited for full user control over the render pipeline.
*/
class RenderPass
{
protected:
    VkExtent2D m_extent;

    VkRenderPass m_obj;

    std::vector<Attachment> m_attachments;
    std::vector<VkFramebuffer> m_framebuffers;
    uint32_t m_framebufferCount;      // How many framebuffers will be attached to this renderpass, usually is just one.
    uint32_t m_framebufferImageDepth; // The depth of the framebuffer image layers.
    std::unordered_map<std::string, ShaderPass *> m_shaderPasses;

    bool m_initiatized{false};
    bool m_isResizeable{true};
    bool m_enabled{true};
    bool m_isResolve;

    /**
     * Creates the renderpass vulkan object. Should be called on init
     *
     */
    void build(VkDevice &device,
               std::vector<VkSubpassDescription> subpasses,
               std::vector<VkSubpassDependency> dependencies);

    /**
     * Begin render pass. Should be called at the start of the render function
     */
    void begin(VkCommandBuffer &cmd, uint32_t framebufferId = 0, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
    /**
     * End render pass.  Should be called at the end of the render function
     */
    void end(VkCommandBuffer &cmd);

public:
    RenderPass(VkExtent2D extent, uint32_t framebufferCount = 1, uint32_t framebufferDepth = 1, bool isDefault = false) : m_extent(extent),
                                                                                                                          m_framebufferCount(framebufferCount),
                                                                                                                          m_framebufferImageDepth(framebufferDepth),
                                                                                                                          m_isResolve(isDefault) {}

    virtual inline void set_active(const bool s)
    {
        m_enabled = s;
    }
    virtual inline bool is_active() { return m_enabled; }

    inline VkExtent2D get_extent() const { return m_extent; }
    inline void set_extent(VkExtent2D extent)
    {
        if (m_isResizeable)
            m_extent = extent;
    }
    inline std::vector<VkFramebuffer> const get_framebuffers() const
    {
        return m_framebuffers;
    }
    inline std::vector<Attachment> &get_attachments()
    {
        return m_attachments;
    }
    inline void set_attachment_clear_value(VkClearValue value, size_t attachmentLayout = 0) { m_attachments[attachmentLayout].clearValue = value; }
    inline VkRenderPass get_obj() const { return m_obj; }
    inline bool is_resizeable() const { return m_isResizeable; }
    inline void set_resizeable(bool op) { m_isResizeable = op; }
    /**
     * Check if its the renderpass that directly renders onto the backbuffer (swapchain present image).
     */
    inline bool is_resolve() const { return m_isResolve; }
    inline bool is_initialized() const { return m_initiatized; }
    inline std::unordered_map<std::string, ShaderPass *> const get_shaderpasses() const { return m_shaderPasses; }

    /*
    Configures and creates the renderpass. Rendeerer will call this functuion when necessary
    */
    virtual void init(VkDevice &device) = 0;

    /*
    Configures and creates the shaderpasses subscribed to the renderpass
    */
    virtual void init_shaderpasses(VkDevice &device, DescriptorManager &descriptorManager) = 0;
    /*
    Render
    */
    virtual void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t framebufferIndex = 0) = 0;
    /**
     * Create framebuffers and images attached to them necessary for the renderpass to work. It also sets the extent of the renderpass.
     */
    virtual void create_framebuffer(VkDevice &device, VmaAllocator &memory, Swapchain *swp = nullptr);
    /**
     * Destroy framebuffers and images attached to them necessary for the renderpass to work. If images have a sampler attached to them, contol the destruction of it too.
     */
    virtual void clean_framebuffer(VkDevice &device, VmaAllocator &memory, bool destroyImageSamplers = true);
    /**
     * Recreates the renderpass with new parameters. Useful for example, when resizing the screen. It automatically manages framebuffer cleanup and regeneration
     *
     */
    virtual void update(VkDevice &device, VmaAllocator &memory, uint32_t layers = 1, uint32_t count = 1, Swapchain *swp = nullptr);
    /**
     * Destroy the renderpass and its shaderpasses. Framebuffers are managed in a sepparate function for felxibilty matters
     */
    virtual void cleanup(VkDevice &device);

    // Utility methods
    static void set_viewport(VkCommandBuffer &cmd, VkExtent2D extent, float minDepth = 0.0f, float maxDepth = 1.0f,
                             float x = 0.0f, float y = 0.0f, int offsetX = 0, int offsetY = 0);
};

VULKAN_ENGINE_NAMESPACE_END

#endif
