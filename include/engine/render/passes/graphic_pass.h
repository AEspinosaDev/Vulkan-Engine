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
#include <engine/render/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

/*
Core abstract GRAPHIC class needed for a renderer to work. It draws polygon primitives and rasterized them onto
framebuffers. It controls the flow of the renderer state, what information and how it is being rendered. It also gives
access to the framebuffers containing the rendered data. It can be inherited for full user control over the render
pipeline.
*/
class BaseGraphicPass : public BasePass
{
protected:
    Graphics::RenderPass               m_renderpass = {};
    std::vector<Graphics::Framebuffer> m_framebuffers;
    uint32_t                           m_framebufferImageDepth; // In case if multilayered rendering.

    virtual void setup_out_attachments( std::vector<Graphics::AttachmentConfig>&  attachments,
                                        std::vector<Graphics::SubPassDependency>& dependencies ) = 0;

public:
    BaseGraphicPass( const ptr<Graphics::Device>&     device,
                     const ptr<Render::GPUResourcePool>& shared,
                     Extent2D                         extent,
                     uint32_t                         framebufferCount = 1,
                     uint32_t                         framebufferDepth = 1,
                     bool                             isResizeable     = true,
                     bool                             isDefault        = false,
                     std::string                      name             = "GRAPHIC PASS" )

        : BasePass( device, shared, extent, true, isResizeable, isDefault, name )
        , m_framebufferImageDepth( framebufferDepth ) {
        !isDefault ? m_framebuffers.resize( framebufferCount )
                   : m_framebuffers.resize( device->get_swapchain().get_present_images().size() );
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
    void         setup( std::vector<Graphics::Frame>& frames ) override;
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

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
