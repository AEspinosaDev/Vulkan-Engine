/*
        This file is part of Vulkan-Engine, a simple to use Vulkan based 3D
   library

        MIT License

        Copyright (c) 2023 Antonio Espinosa Garcia

        ////////////////////////////////////////////////////////////////////////////////////

        Implementation of this class is fragmentated in three submodules:

        * vk_renderer.cpp
        * vk_renderer_data_mgr.cpp

        ////////////////////////////////////////////////////////////////////////////////////
*/
#ifndef RENDERER_H
#define RENDERER_H

#include <engine/common.h>

#include <engine/core/materials/material.h>
#include <engine/core/renderpasses/irradiance_compute_pass.h>
#include <engine/core/renderpasses/panorama_conversion_pass.h>
#include <engine/core/textures/texture.h>
#include <engine/core/textures/textureLDR.h>
#include <engine/core/windows/window.h>
#include <engine/core/windows/windowGLFW.h>

#include <engine/graphics/device.h>
#include <engine/graphics/image.h>
#include <engine/graphics/uniforms.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/utilities/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renderer Global Settings Data
*/
struct RendererSettings {

    MSAASamples     samplesMSAA          = MSAA_x4;
    BufferingType   bufferingType        = DOUBLE_BUFFER;
    SyncType        screenSync           = MAILBOX_SYNC;
    ColorFormatType colorFormat          = SBGRA_8;
    DepthFormatType depthFormat          = D32F;
    uint16_t        irradianceResolution = 128;
    Vec4            clearColor           = Vec4{0.0, 0.0, 0.0, 1.0};
    bool            autoClearColor       = true;
    bool            autoClearDepth       = true;
    bool            autoClearStencil     = true;
    bool            enableUI             = false;
    bool            enableRaytracing     = true;
};
/*
Structure that contains a list of renderpasses. These renderpasses will render
in the order they where added.
*/
struct RenderPipeline {
    std::vector<Core::RenderPass*> renderpasses;

    // Auxiliar passes
    Core::PanoramaConverterPass*  panoramaConverterPass{nullptr};
    Core::IrrandianceComputePass* irradianceComputePass{nullptr};

    void push_renderpass(Core::RenderPass* pass) {
        renderpasses.push_back(pass);
    };
    void render(Graphics::Frame& currentFrame, VKFW::Core::Scene* scene, uint32_t presentImageIndex = 0U) {
        for (Core::RenderPass* pass : renderpasses)
        {
            if (pass->is_active())
                pass->render(currentFrame, scene, presentImageIndex);
        }
    }
    void flush() {
        for (Core::RenderPass* pass : renderpasses)
        {
            pass->cleanup();
        }
        if (panoramaConverterPass)
            panoramaConverterPass->cleanup();
        if (irradianceComputePass)
            irradianceComputePass->cleanup();
    }
    void flush_framebuffers() {
        for (Core::RenderPass* pass : renderpasses)
        {
            pass->clean_framebuffer();
        }
        if (panoramaConverterPass)
            panoramaConverterPass->clean_framebuffer();
        if (irradianceComputePass)
            irradianceComputePass->clean_framebuffer();
    }
};

/**
 * Basic class. Renders a given scene data to a given window. Fully
 * parametrizable. It has to be inherited for achieving a higher end
 * application.
 */
class BaseRenderer
{
#pragma region Properties
  protected:
    Graphics::Device             m_device{};
    std::vector<Graphics::Frame> m_frames;

    Core::IWindow*   m_window;
    Core::Mesh*      m_vignette{};
    RendererSettings m_settings{};
    RenderPipeline   m_renderPipeline;

    Graphics::Utils::DeletionQueue m_deletionQueue;

    // Query
    uint32_t m_currentFrame{0};
    bool     m_initialized{false};
    bool     m_updateFramebuffers{false};

#pragma endregion
  public:
    BaseRenderer(Core::IWindow* window)
        : m_window(window) {
        on_instance();
    }
    BaseRenderer(Core::IWindow* window, RendererSettings settings)
        : m_window(window)
        , m_settings(settings) {
        on_instance();
    }

#pragma region Getters & Setters

    inline Core::IWindow* const get_window() const {
        return m_window;
    }

    inline RendererSettings get_settings() {
        return m_settings;
    }
    inline void set_settings(RendererSettings settings) {
        m_settings = settings;
    }
    inline void set_clearcolor(Vec4 c) {
        m_settings.clearColor = c;
    }
    inline void set_antialiasing(MSAASamples msaa) {
        m_settings.samplesMSAA = msaa;
    }
    inline void set_color_format(ColorFormatType color) {
        m_settings.colorFormat = color;
    }
    inline void set_depth_format(DepthFormatType d) {
        m_settings.depthFormat = d;
    }
    inline void set_enable_gui(bool op) {
        m_settings.enableUI = op;
    }
    inline RenderPipeline get_render_pipeline() const {
        return m_renderPipeline;
    }
    inline void enable_gui_overlay(bool op) {
        m_settings.enableUI;
    }
    inline void set_sync_type(SyncType sync) {
        m_settings.screenSync = sync;
        if (m_initialized)
            m_updateFramebuffers = true;
    }

#pragma endregion
#pragma region Public Functions

    /**
     * Inits the renderer.
     */
    void init();

    /**
     * Standalone pre-implemented render loop for the renderer.
     */
    void run(Core::Scene* const scene);
    /**
     * Renders a given scene.
     */
    void render(Core::Scene* const scene);
    /**
     * Shut the renderer down.
     */
    void shutdown(Core::Scene* const scene);

#pragma endregion
#pragma region Core Functions
  protected:
    /*
     Init renderpasses and create framebuffers and image resources attached to them
     */
    virtual void setup_renderpasses();
    /*
    What to do when instancing the renderer
    */
    virtual void on_instance() {
    }
    /*
    What to do when initiating the renderer
    */
    virtual void on_init() {
    }
    /*
    What to do just before rendering
    */
    virtual void on_before_render(Core::Scene* const scene);
    /*
    What to do just before rendering
    */
    virtual void on_after_render(RenderResult& renderResult, Core::Scene* const scene);
    /*
    What to do when shutting down the renderer
    */
    virtual void on_shutdown(Core::Scene* const scene) {
    }
    /*
    Link images of previous passes to current pass
    */
    void connect_renderpass(Core::RenderPass* const currentPass);
    /*
    Clean and recreates swapchain and framebuffers in the renderer. Useful to use
    when resizing context
    */
    void update_renderpasses();

#pragma endregion
    /*
            ////////////////////////////////////////////////////////////////////////////////////

            Implementation of this region can be found in the module ==>>
       vk_renderer_data_mgr.cpp

            ////////////////////////////////////////////////////////////////////////////////////
    */
#pragma region Data Management
    /*
    Resource like samplers, base textures and misc creation
    */
    virtual void init_resources();
    /*
    Clean all resources used
    */
    virtual void clean_Resources();
    /*
    Global descriptor layouts uniforms buffer upload to GPU
    */
    virtual void update_global_data(Core::Scene* const scene);
    /*
    Object descriptor layouts uniforms buffer upload to GPU
    */
    virtual void update_object_data(Core::Scene* const scene);
    /*
    Initialize and setup texture IMAGE
    */
    void upload_texture_image(Core::ITexture* const t);
    void destroy_texture_image(Core::ITexture* const t);
    /*
    Upload geometry vertex buffers to the GPU
    */
    void upload_geometry_data(Core::Geometry* const g, bool createAccelStructure = true);
    void destroy_geometry_data(Core::Geometry* const g);
    /*
    Setup skybox
    */
    void setup_skybox(Core::Scene* const scene);

#pragma region GUI
    /*
    Initialize gui layout in case ther's one enabled
    */
    void init_gui();

#pragma endregion
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif // VK_RENDERER