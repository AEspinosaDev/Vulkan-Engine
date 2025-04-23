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

#include <engine/core/passes/pass.h>
#include <engine/core/scene/scene.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
Core abstract GRAPHIC class needed for a renderer to work. It draws polygon primitives and rasterized them onto
framebuffers. It controls the flow of the renderer state, what information and how it is being rendered. It also gives
access to the framebuffers containing the rendered data. It can be inherited for full user control over the render
pipeline.
*/
template <std::size_t numberIN, std::size_t numberOUT> class BaseGraphicPass : public BasePass
{
  protected:
    Graphics::RenderPass               m_renderpass = {};
    std::vector<Graphics::Framebuffer> m_framebuffers;
    uint32_t                           m_framebufferImageDepth; // In case if multilayered rendering.

    virtual void setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                                       std::vector<Graphics::SubPassDependency>& dependencies) = 0;

  public:
    BaseGraphicPass(Graphics::Device*                      device,
                    const PassConfig<numberIN, numberOUT>& config,
                    Extent2D                               extent,
                    uint32_t                               framebufferCount = 1,
                    uint32_t                               framebufferDepth = 1,
                    std::string                            name             = "GRAPHIC PASS")

        : BasePass(device, extent, config.graphical, config.resizeable, config.isDefault, name)
        , m_framebufferImageDepth(framebufferDepth) {
        BasePass::store_attachments<numberIN, numberOUT>(config);
        !config.isDefault ? m_framebuffers.resize(framebufferCount)
                          : m_framebuffers.resize(device->get_swapchain().get_present_images().size());
    }
    virtual ~BaseGraphicPass() {
    }

#pragma region Getters & Setters

    inline Graphics::RenderPass get_renderpass() const {
        return m_renderpass;
    }
    inline std::vector<Graphics::Framebuffer> const get_framebuffers() const {
        return m_framebuffers;
    }

#pragma endregion
#pragma region Core Functions
    /*
    Setups de renderpass. Init, create framebuffers, pipelines and resources ...
    */
    void         setup(std::vector<Graphics::Frame>& frames) override;
    virtual void resize_attachments() override;
    virtual void cleanup() override;
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

#pragma endregion
};

#pragma region Implementation

template <std::size_t numberIN, std::size_t numberOUT>
void BaseGraphicPass<numberIN, numberOUT>::setup(std::vector<Graphics::Frame>& frames) {
    std::vector<Graphics::AttachmentConfig>  attachments;
    std::vector<Graphics::SubPassDependency> dependencies;
    setup_out_attachments(attachments, dependencies);

    if (!attachments.empty())
    {
        m_renderpass = m_device->create_render_pass(attachments, dependencies);
        m_renderpass.set_debug_name(m_name.c_str());
    } else
        m_isGraphical = false;
    m_initiatized = true;

    create_framebuffer();
    setup_uniforms(frames);
    setup_shader_passes();
}
template <std::size_t numberIN, std::size_t numberOUT> void BaseGraphicPass<numberIN, numberOUT>::cleanup() {
    m_renderpass.cleanup();
    clean_framebuffer();
    BasePass::cleanup();
}
template <std::size_t numberIN, std::size_t numberOUT> void BaseGraphicPass<numberIN, numberOUT>::create_framebuffer() {
    if (!m_initiatized)
        return;

    for (size_t fb = 0; fb < m_framebuffers.size(); fb++)
    {
        m_framebuffers[fb] =
            m_device->create_framebuffer(m_renderpass, m_outAttachments, m_imageExtent, m_framebufferImageDepth, fb);
    }
}
template <std::size_t numberIN, std::size_t numberOUT> void BaseGraphicPass<numberIN, numberOUT>::clean_framebuffer() {
    if (!m_initiatized)
        return;
    for (Graphics::Framebuffer& fb : m_framebuffers)
        fb.cleanup();

    for (size_t i = 0; i < m_interAttachments.size(); i++)
    {
        m_interAttachments[i].cleanup();
    }

    for (size_t i = 0; i < m_outAttachments.size(); i++)
    {
        m_outAttachments[i]->cleanup();
    }
}
template <std::size_t numberIN, std::size_t numberOUT> void BaseGraphicPass<numberIN, numberOUT>::resize_attachments() {
    if (!m_initiatized)
        return;

    clean_framebuffer();
    create_framebuffer();
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
