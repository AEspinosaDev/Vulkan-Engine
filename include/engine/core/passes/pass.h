/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PASS_H
#define PASS_H

#include <array>

#include <engine/common.h>

#include <engine/graphics/descriptors.h>
#include <engine/graphics/device.h>
#include <engine/graphics/frame.h>
#include <engine/graphics/framebuffer.h>
#include <engine/graphics/renderpass.h>
#include <engine/graphics/swapchain.h>

#include <engine/core/scene/scene.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
Core abstract class needed for a renderer to work.
It controls the flow of the renderer state, what information and how it is being
rendered/computed. It also gives access to the framebuffers containing the rendered data.
It can be inherited for full user control over the render/compute pipeline.
*/
class BasePass;
typedef BasePass GraphicPass;   // Sintax for graphic passes that draw primitives and rasterized them onto framebuffers
typedef BasePass ComputePass;   // Sintax for passes focused on GPGPU
typedef BasePass RaytracedPass; // Sintax for passes that only use the raytracing mode.
class BasePass
{
  protected:
    // Graphic Objects
    Graphics::Device*                                          m_device         = nullptr;
    Graphics::DescriptorPool                                   m_descriptorPool = {};
    std::unordered_map<std::string, Graphics::BaseShaderPass*> m_shaderPasses;
    // In case it is a graphical pass ... which is the most usual
    Graphics::RenderPass               m_renderpass = {};
    std::vector<Graphics::Framebuffer> m_framebuffers;
    uint32_t                           m_framebufferImageDepth; // In case if multilayered rendering.

    std::string m_name;

    // Key: Renderpass ID
    // Value: Framebuffer's image ID inside renderpass
    std::unordered_map<uint32_t, std::vector<uint32_t>> m_imageDepedanceTable;

    // Query
    bool m_initiatized  = false;
    bool m_isResizeable = true;
    bool m_enabled      = true;
    bool m_isDefault    = false;
    bool m_isGraphical  = true;

    virtual void
                 setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                                   std::vector<Graphics::SubPassDependency>& dependencies) = 0; // Only for graphical renderpasses
    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames) = 0;
    virtual void setup_shader_passes()                                = 0;

  public:
    BasePass(Graphics::Device* ctx,
             Extent2D          extent,
             uint32_t          framebufferCount = 1,
             uint32_t          framebufferDepth = 1,
             bool              isDefault        = false,
             std::string       name             = "Graphic Pass")
        : m_device(ctx)
        , m_framebufferImageDepth(framebufferDepth)
        , m_isDefault(isDefault)
        , m_name(name) {
        m_framebuffers.resize(framebufferCount);
        m_renderpass.extent = extent;
    }

#pragma region Getters & Setters

    virtual inline void set_active(const bool s) {
        m_enabled = s;
    }
    virtual inline bool is_active() {
        return m_enabled;
    }

    inline Extent2D get_extent() const {
        return m_renderpass.extent;
    }
    inline void set_extent(Extent2D extent) {
        m_renderpass.extent = extent;
    }

    inline Graphics::RenderPass get_handle() const {
        return m_renderpass;
    }
    inline std::vector<Graphics::Framebuffer> const get_framebuffers() const {
        return m_framebuffers;
    }

    inline std::vector<Graphics::Attachment> get_attachments() {
        return m_renderpass.attachments;
    }
    inline void set_attachment_clear_value(VkClearValue value, size_t attachmentLayout = 0) {
        m_renderpass.attachments[attachmentLayout].clearValue = value;
    }

    inline bool resizeable() const {
        return m_isResizeable;
    }
    inline void set_resizeable(bool op) {
        m_isResizeable = op;
    }
    /**
     * Check if its the renderpass that directly renders onto the backbuffer
     * (swapchain present image).
     */
    inline bool default_pass() const {
        return m_isDefault;
    }
    inline bool initialized() const {
        return m_initiatized;
    }
    inline bool is_graphical() const {
        return m_isGraphical;
    }

    inline std::unordered_map<std::string, Graphics::ShaderPass*> const get_shaderpasses() const {
        return m_shaderPasses;
    }

    /*
    Sets a table of connection with different passes. Key is the pass ID and value
    is the atachment number
    */
    inline void set_image_dependace_table(std::unordered_map<uint32_t, std::vector<uint32_t>> table) {
        m_imageDepedanceTable = table;
    }
    inline std::unordered_map<uint32_t, std::vector<uint32_t>> get_image_dependace_table() const {
        return m_imageDepedanceTable;
    }

#pragma endregion
#pragma region Core Functions
    /*
    Setups de renderpass. Init, create framebuffers, pipelines and resources ...
    */
    void setup(std::vector<Graphics::Frame>& frames);

    virtual void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0) = 0;

    virtual void update_uniforms(uint32_t frameIndex, Scene* const scene) {
    }
    virtual void connect_to_previous_images(std::vector<Graphics::Image> images) {
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

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
