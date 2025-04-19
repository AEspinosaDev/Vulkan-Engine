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
Data containing a dependicy image's location belonging to a previows pass
*/
struct ImageDependency;

/*
Core abstract class needed for a renderer to work.
It controls the flow of the renderer state, what information and how it is being
rendered/computed.
It can be inherited for full user control over the render/compute pipeline.
*/
class BasePass;
typedef BasePass ComputePass; // Sintax for passes focused on GPGPU
class BasePass
{
  protected:
    // Graphic Objects
    Graphics::Device*                                          m_device         = nullptr;
    Graphics::DescriptorPool                                   m_descriptorPool = {};
    std::unordered_map<std::string, Graphics::BaseShaderPass*> m_shaderPasses;
    // In case is not graphical or need auxiliar data, other graphic data can be stored onto these images
    std::vector<Graphics::Image> m_resourceImages;

    Extent2D                     m_imageExtent;
    std::string                  m_name;
    std::vector<ImageDependency> m_imageDependencies; // Previous passes image dependency list

    // Query
    bool m_initiatized  = false;
    bool m_isResizeable = true;
    bool m_enabled      = true;
    bool m_isDefault    = false;
    bool m_isGraphical  = true;

    virtual void setup_uniforms(std::vector<Graphics::Frame>& frames) = 0;
    virtual void setup_shader_passes()                                = 0;

  public:
    BasePass(Graphics::Device* ctx, Extent2D extent, bool isDefault = false, std::string name = "UNNAMED PASS")
        : m_device(ctx)
        , m_isDefault(isDefault)
        , m_name(name)
        , m_imageExtent(extent) {
    }
    virtual ~BasePass() {
    }

#pragma region Getters & Setters

    virtual inline void set_active(const bool s) {
        m_enabled = s;
    }
    virtual inline bool is_active() {
        return m_enabled;
    }

    inline Extent2D get_extent() const {
        return m_imageExtent;
    }
    inline void set_extent(Extent2D extent) {
        m_imageExtent = extent;
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
    Sets a vector of depedencies with different passes.
    */
    inline void set_image_dependencies(std::vector<ImageDependency> dependencies) {
        m_imageDependencies = dependencies;
    }
    inline std::vector<ImageDependency> get_image_dependencies() const {
        return m_imageDependencies;
    }
    inline std::vector<Graphics::Image> const get_resource_images() const {
        return m_resourceImages;
    }

#pragma endregion
#pragma region Core Functions
    /*
    Setups de renderpass. Init, create framebuffers, pipelines and resources ...
    */
    virtual void setup(std::vector<Graphics::Frame>& frames);

    virtual void render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex = 0) = 0;

    virtual void update_uniforms(uint32_t frameIndex, Scene* const scene) {
    }
    virtual void resize_attachments() {
    }
    virtual void link_previous_images(std::vector<Graphics::Image> images) {
    }
    virtual void cleanup();
#pragma endregion
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
