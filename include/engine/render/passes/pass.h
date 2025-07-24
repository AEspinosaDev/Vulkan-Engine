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

#include <engine/render/GPU_resource_pool.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

using namespace Core;
using namespace Graphics;

namespace Render {

#define CHECK_INITIALIZATION()  \
    if ( !this->m_initiatized ) \
        return;

#pragma region Config
/*
Pass config <Input Attachments, Output Attachmetns>
*/
template <std::size_t numberIN, std::size_t numberOUT>
struct PassLinkage {
    // Connectivity Info
    std::vector<Graphics::Image>&   attachmentPool;
    std::array<uint32_t, numberIN>  inAttachmentsIdx;
    std::array<uint32_t, numberOUT> outAttachmentsIdx;
};

/*
Core abstract class needed for a renderer to work.
It controls the flow of the renderer state, what information and how it is being
rendered/computed.
It can be inherited for full user control over the render/compute pipeline.
*/
class BasePass
{
protected:
    // Graphic Objects
    ptr<Graphics::Device>                                      m_device;
    Graphics::DescriptorPool                                   m_descriptorPool = {};
    std::unordered_map<std::string, Graphics::BaseShaderPass*> m_shaderPasses;

    // Shared resources
    ptr<Render::GPUResourcePool> m_shared;

    // Attachment Images
    std::vector<Graphics::Image*> m_inAttachments;
    std::vector<Graphics::Image>  m_interAttachments; // Intermidiate (internal)
    std::vector<Graphics::Image*> m_outAttachments;

    // Params
    Extent2D    m_imageExtent;
    std::string m_name;
    // Query params
    bool m_initiatized  = false;
    bool m_isResizeable = true;
    bool m_enabled      = true;
    bool m_isDefault    = false;
    bool m_isGraphical  = true;

    virtual void setup_uniforms( std::vector<Graphics::Frame>& frames ) = 0;
    virtual void setup_shader_passes()                                  = 0;

    template <std::size_t numberIN, std::size_t numberOUT>
    inline void store_attachments( const PassLinkage<numberIN, numberOUT>& linkage ) {

        // Populate pass attachments vector from the attachment pool given their idx
        m_inAttachments.resize( numberIN, nullptr );
        m_outAttachments.resize( numberOUT, nullptr );
        for ( size_t i = 0; i < numberIN; i++ )
        {
            m_inAttachments[i] = &linkage.attachmentPool[linkage.inAttachmentsIdx[i]];
        }
        for ( size_t i = 0; i < numberOUT; i++ )
        {
            m_outAttachments[i] = &linkage.attachmentPool[linkage.outAttachmentsIdx[i]];
        }
    }

public:
    BasePass( const ptr<Graphics::Device>&     device,
              const ptr<Render::GPUResourcePool>& shared,
              Extent2D                         extent,
              bool                             isGraphical,
              bool                             isResizeable,
              bool                             isDefault,
              std::string                      name = "UNNAMED PASS" );

    virtual ~BasePass() {
    }

#pragma region Getters & Setters

    virtual inline void set_active( const bool s ) {
        m_enabled = s;
    }
    virtual inline bool is_active() {
        return m_enabled;
    }

    inline Extent2D get_extent() const {
        return m_imageExtent;
    }
    inline void set_extent( Extent2D extent ) {
        m_imageExtent = extent;
    }

    inline bool resizeable() const {
        return m_isResizeable;
    }
    inline void set_resizeable( bool op ) {
        m_isResizeable = op;
    }
    /**
     * Check if its the pass that directly renders onto the backbuffer (swapchain)
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
    inline std::vector<Graphics::Image*> get_output_attachments() const {
        return m_outAttachments;
    }
    inline std::vector<Graphics::Image*> get_input_attachments() const {
        return m_inAttachments;
    }
    inline void set_attachment_clear_value( ClearValue value, size_t attachmentIdx = 0 ) {
        m_outAttachments[attachmentIdx]->config.clearValue = value;
    }

#pragma endregion
#pragma region Core Functions
    virtual void setup( std::vector<Graphics::Frame>& frames );

    virtual void execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0 ) = 0;

    virtual void update_uniforms( uint32_t frameIndex, Scene* const scene ) {
    }
    virtual void resize_attachments() {
    }
    virtual void link_input_attachments() {
    }
    virtual void cleanup();
#pragma endregion
  
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif
