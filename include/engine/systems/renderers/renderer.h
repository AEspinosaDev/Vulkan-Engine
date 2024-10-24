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
#include <engine/core/renderpasses/renderpass.h>
#include <engine/core/textures/texture.h>
#include <engine/core/windows/window.h>
#include <engine/core/windows/windowGLFW.h>

#include <engine/graphics/bootstrap.h>
#include <engine/graphics/context.h>
#include <engine/graphics/image.h>
#include <engine/graphics/initializers.h>
#include <engine/graphics/uniforms.h>
#include <engine/graphics/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems
{

/*
Renderer Global Settings Data
*/
struct RendererSettings
{
    MSAASamples samplesMSAA{MSAA_x4};
    BufferingType bufferingType{_DOUBLE};
    SyncType screenSync{MAILBOX_SYNC};
    ColorFormatType colorFormat{SBGRA_8};
    DepthFormatType depthFormat{D32F};

    Vec4 clearColor{Vec4{0.0, 0.0, 0.0, 1.0}};

    bool autoClearColor{true};
    bool autoClearDepth{true};
    bool autoClearStencil{true};

    bool enableUI{false};
};
/*
Structure that contains a list of renderpasses. These renderpasses will render
in the order they where added.
*/
struct RenderPipeline
{
    std::vector<Core::RenderPass *> renderpasses;

    void push_renderpass(Core::RenderPass *pass)
    {
        renderpasses.push_back(pass);
    };
};

/**
 * Virtual class. Renders a given scene data to a given window. Fully
 * parametrizable. It has to be inherited for achieving a higher end
 * application.
 */
class RendererBase
{
#pragma region Properties
  protected:
    Graphics::Context m_context{};
    Core::WindowBase *m_window;

    RendererSettings m_settings{};
    RenderPipeline m_renderPipeline;

    Graphics::utils::DeletionQueue m_deletionQueue;

    // Query
    uint32_t m_currentFrame{0};
    bool m_initialized{false};
    bool m_updateFramebuffers{false};

#pragma endregion
  public:
    RendererBase(Core::WindowBase *window) : m_window(window)
    {
        on_instance();
    }
    RendererBase(Core::WindowBase *window, RendererSettings settings) : m_window(window), m_settings(settings)
    {
        on_instance();
    }

#pragma region Getters & Setters

    inline Core::WindowBase *const get_window() const
    {
        return m_window;
    }

    inline RendererSettings get_settings()
    {
        return m_settings;
    }
    inline void set_settings(RendererSettings settings)
    {
        m_settings = settings;
    }

    inline void set_clearcolor(Vec4 c)
    {
        m_settings.clearColor = c;
    }
    inline void set_antialiasing(MSAASamples msaa)
    {
        m_settings.samplesMSAA = msaa;
    }
    inline void set_color_format(ColorFormatType color)
    {
        m_settings.colorFormat = color;
    }
    inline void set_depth_format(DepthFormatType d)
    {
        m_settings.depthFormat = d;
    }
    inline void set_enable_gui(bool op)
    {
        m_settings.enableUI = op;
    }

    inline RenderPipeline get_render_pipeline() const
    {
        return m_renderPipeline;
    }

    inline void enable_gui_overlay(bool op)
    {
        m_settings.enableUI;
    }
    inline void set_sync_type(SyncType sync)
    {
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
    void run(Core::Scene *const scene);
    /**
     * Renders a given scene.
     */
    void render(Core::Scene *const scene);
    /**
     * Shut the renderer down.
     */
    void shutdown(Core::Scene *const scene);

#pragma endregion
#pragma region Core Functions
  protected:
    /*
     Init renderpasses and create framebuffers and image resources attached to them
     */
    virtual void setup_renderpasses() = 0;
    /*
    What to do when instancing the renderer
    */
    virtual void on_instance()
    {
    }
    /*
    What to do when initiating the renderer
    */
    virtual void on_init()
    {
    }
    /*
    What to do just before rendering
    */
    virtual void on_before_render(Core::Scene *const scene);
    /*
    What to do just before rendering
    */
    virtual void on_after_render(VkResult &renderResult, Core::Scene *const scene);
    /*
    What to do when shutting down the renderer
    */
    virtual void on_shutdown(Core::Scene *const scene)
    {
    }
    /*
    Link images of previous passes to current pass
    */
    void connect_renderpass(Core::RenderPass *const currentPass);
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
    virtual void update_global_data(Core::Scene *const scene);
    /*
    Object descriptor layouts uniforms buffer upload to GPU
    */
    virtual void update_object_data(Core::Scene *const scene);
    /*
    Initialize and setup textures and uniforms in given material
    */
    virtual void upload_material_textures(Core::IMaterial *const mat);
    /*
    Upload geometry vertex buffers to the GPU
    */
    void upload_geometry_data(Core::Geometry *const g);

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