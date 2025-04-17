/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GRAPHIC_PASS_H
#define GRAPHIC_PASS_H

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
Data containing a dependicy image's location belonging to a previows pass
*/
struct ImageDependency;

/*
Core abstract GRAPHIC class needed for a renderer to work. It draws polygon primitives and rasterized them onto
framebuffers. It controls the flow of the renderer state, what information and how it is being rendered. It also gives
access to the framebuffers containing the rendered data. It can be inherited for full user control over the render
pipeline.
*/
class GraphicPass : public BasePass
{
  protected:
    Graphics::RenderPass               m_renderpass = {};
    std::vector<Graphics::Framebuffer> m_framebuffers;
    uint32_t                           m_framebufferImageDepth; // In case if multilayered rendering.

    virtual void
                 setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                   std::vector<Graphics::SubPassDependency>& dependencies) = 0; 

  public:
  GraphicPass(Graphics::Device* ctx,
             Extent2D          extent,
             uint32_t          framebufferCount = 1,
             uint32_t          framebufferDepth = 1,
             bool              isDefault        = false,
             std::string       name             = "UNNAMED PASS")
        : m_device(ctx)
        , m_framebufferImageDepth(framebufferDepth)
        , m_isDefault(isDefault)
        , m_name(name) {
        !isDefault ? m_framebuffers.resize(framebufferCount)
                   : m_framebuffers.resize(m_device->get_swapchain().get_present_images().size());
        m_imageExtent = extent;
    }
    virtual ~GraphicPass() {
    }

#pragma region Getters & Setters

   

    inline Graphics::RenderPass get_renderpass() const {
        return m_renderpass;
    }
    inline std::vector<Graphics::Framebuffer> const get_framebuffers() const {
        return m_framebuffers;
    }
    inline std::vector<Graphics::Image> const get_resource_images() const {
        return m_resourceImages;
    }
    inline void set_attachment_clear_value(VkClearValue value, size_t attachmentLayout = 0) {
        m_renderpass.attachmentsInfo[attachmentLayout].clearValue = value;
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
    virtual void link_previous_images(std::vector<Graphics::Image> images) {
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
    virtual void update_framebuffer();
    /**
     * Destroy the renderpass and its shaderpasses. Framebuffers are managed in a
     * sepparate function for felxibilty matters
     */
    virtual void cleanup();
#pragma endregion

    /*
    Public static member.
    Vignette for rendering textures onto screen.*/
    static Core::Geometry* vignette;
};

#pragma region IMAGE DEP

struct ImageDependency {
    uint32_t passID = 0; // The pass that produces this image
    uint32_t fboID  = 0; // The FBO within the pass that produces this image
    bool isFBO = true;   // If set to false, It will take the attachments from the pass resourceImages (Useful if not a
                         // graphical pass).
    std::vector<uint32_t> attachmentIDs; // The attachment indeces within the FBO

    ImageDependency(uint32_t passId, u_int fboId, std::vector<uint32_t> attachmentIds)
        : passID(passId)
        , fboID(fboId)
        , attachmentIDs(attachmentIds) {
    }
    ImageDependency(uint32_t passId, std::vector<uint32_t> attachmentIds)
        : passID(passId)
        , attachmentIDs(attachmentIds)
        , isFBO(false) {
    }
};

#pragma endregion
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
