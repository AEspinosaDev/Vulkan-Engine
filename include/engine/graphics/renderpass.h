/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <array>

#include <engine/common.h>

#include <engine/graphics/context.h>
#include <engine/graphics/descriptors.h>
#include <engine/graphics/frame.h>
#include <engine/graphics/swapchain.h>

#include <engine/core/scene/scene.h>

#include <engine/utilities/gui.h>


VULKAN_ENGINE_NAMESPACE_BEGIN

/*
Attachment infor needed for using a renderpasses and its framebuffers.
*/
struct Attachment
{
    Image image{};

    VkClearValue clearValue{};

    bool isPresentImage{false};

     Attachment(ImageConfig config, ViewConfig viewConfig, SamplerConfig samplerConfig, VkClearValue clearVal = {{{0.0, 0.0, 0.0, 1.0}}})
        : clearValue(clearVal)
    {
        image.config = config;
        image.viewConfig = viewConfig;
        image.samplerConfig = samplerConfig;
        clearValue.depthStencil.depth = 1.0f;
    }; 
};

/*
Core abstract class needed for rendering.
It controls the flow of the render state, what information and how it is being
rendered. It also gives access to the framebuffers containing the rendered data.
It can be inherited for full user control over the render pipeline.
*/
class RenderPass
{
  protected:
    Context *m_context{nullptr};

    VkRenderPass m_handle;
    VkExtent2D m_extent;

    uint32_t m_framebufferCount;      // How many framebuffers will be attached to this
                                      // renderpass, usually is just one.
    uint32_t m_framebufferImageDepth; // The depth of the framebuffer image layers.
    std::vector<VkFramebuffer> m_framebuffer_handles;

    std::vector<Attachment> m_attachments;

    std::unordered_map<std::string, ShaderPass *> m_shaderPasses;

    DescriptorManager m_descriptorManager{};

    // Key: Renderpass ID
    // Value: Framebuffer's image ID inside renderpass
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_imageDepedanceTable;

    bool m_initiatized{false};
    bool m_isResizeable{true};
    bool m_enabled{true};
    bool m_isDefault;

    /**
     * Begin render pass. Should be called at the start of the render function
     */
    void begin(VkCommandBuffer &cmd, uint32_t framebufferId = 0,
               VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
    /**
     * End render pass.  Should be called at the end of the render function
     */
    void end(VkCommandBuffer &cmd);

    /**
     * Draw given geometry
     */
    void draw(VkCommandBuffer &cmd, Geometry *g);

  public:
    RenderPass(Context *ctx, VkExtent2D extent, uint32_t framebufferCount = 1, uint32_t framebufferDepth = 1,
               bool isDefault = false)
        : m_context(ctx), m_extent(extent), m_framebufferCount(framebufferCount),
          m_framebufferImageDepth(framebufferDepth), m_isDefault(isDefault)
    {
    }

#pragma region Getters & Setters

    virtual inline void set_active(const bool s)
    {
        m_enabled = s;
    }
    virtual inline bool is_active()
    {
        return m_enabled;
    }

    inline VkExtent2D get_extent() const
    {
        return m_extent;
    }
    inline void set_extent(VkExtent2D extent)
    {
        m_extent = extent;
    }

    inline VkRenderPass get_handle() const
    {
        return m_handle;
    }
    inline std::vector<VkFramebuffer> const get_framebuffers_handle() const
    {
        return m_framebuffer_handles;
    }

    inline std::vector<Attachment> get_attachments()
    {
        return m_attachments;
    }
    inline void set_attachment_clear_value(VkClearValue value, size_t attachmentLayout = 0)
    {
        m_attachments[attachmentLayout].clearValue = value;
    }

    inline bool resizeable() const
    {
        return m_isResizeable;
    }
    inline void set_resizeable(bool op)
    {
        m_isResizeable = op;
    }
    /**
     * Check if its the renderpass that directly renders onto the backbuffer
     * (swapchain present image).
     */
    inline bool default_pass() const
    {
        return m_isDefault;
    }
    inline bool initialized() const
    {
        return m_initiatized;
    }

    inline std::unordered_map<std::string, ShaderPass *> const get_shaderpasses() const
    {
        return m_shaderPasses;
    }

    /*
    Sets a table of connection with different passes. Key is the pass ID and value
    is the atachment number
    */
    inline void set_image_dependace_table(std::unordered_map<uint32_t, std::vector<uint32_t>> table)
    {
        m_imageDepedanceTable = table;
    }
    inline std::unordered_map<uint32_t, std::vector<uint32_t>> get_image_dependace_table() const
    {
        return m_imageDepedanceTable;
    }

#pragma endregion
#pragma region Core Functions
    /*
    Configures and creates the renderpass. Rendeerer will call this function when
    necessary
    */
    virtual void init() = 0;
    /*
    Use it in case renderpass needs local descriptor sets
    */
    virtual void create_descriptors() = 0;
    /*
    Configures and creates the shaderpasses subscribed to the renderpass
    */
    virtual void create_graphic_pipelines() = 0;
    /*
    Render
    */
    virtual void render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0) = 0;
    /*
    Filling renderpass local uniforms buffers and misc
    */
    virtual void init_resources()
    {
    }
    /*
    Upload data related to renderpass local uniforms and its descriptors sets
    */
    virtual void upload_data(uint32_t frameIndex, Scene *const scene)
    {
    }
    /*
    Update descriptors pointing to past passes image buffers
    */
    virtual void connect_to_previous_images(std::vector<Image> images)
    {
    }
    /**
     * Create framebuffers and images attached to them necessary for the
     * renderpass to work. It also sets the extent of the renderpass.
     */
    virtual void create_framebuffer();
    /**
     * Destroy framebuffers and images attached to them necessary for the
     * renderpass to work. If images have a sampler attached to them, contol the
     * destruction of it too.
     */
    virtual void clean_framebuffer();
    /**
     * Recreates the renderpass with new parameters. Useful for example, when
     * resizing the screen. It automatically manages framebuffer cleanup and
     * regeneration
     *
     */
    virtual void update();
    /**
     * Destroy the renderpass and its shaderpasses. Framebuffers are managed in a
     * sepparate function for felxibilty matters
     */
    virtual void cleanup();
#pragma endregion
};

VULKAN_ENGINE_NAMESPACE_END

#endif
