/*
        This file is part of Vulkan-Engine, a simple to use Vulkan based 3D
   library

        MIT License

        Copyright (c) 2023 Antonio Espinosa Garcia

        ////////////////////////////////////////////////////////////////////////////////////

*/
#ifndef RENDERER_H
#define RENDERER_H

#include <engine/common.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renderer Global Settings Data
*/
struct RendererSettings {

    MSAASamples     samplesMSAA      = MSAASamples::x4;
    BufferingType   bufferingType    = BufferingType::DOUBLE;
    SyncType        screenSync       = SyncType::MAILBOX;
    ColorFormatType colorFormat      = SBGRA_8;
    ColorFormatType depthFormat      = DEPTH_32F;
    Vec4            clearColor       = Vec4{0.0, 0.0, 0.0, 1.0};
    bool            autoClearColor   = true;
    bool            autoClearDepth   = true;
    bool            autoClearStencil = true;
    bool            enableUI         = false;
    bool            enableRaytracing = true;
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

    Core::IWindow* m_window;

    RendererSettings               m_settings{};
    std::vector<Core::RenderPass*> m_renderpasses;

    Graphics::Utils::DeletionQueue m_deletionQueue;

    // Query
    uint32_t m_currentFrame       = 0;
    bool     m_initialized        = false;
    bool     m_updateFramebuffers = false;

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
    inline void set_depth_format(ColorFormatType d) {
        m_settings.depthFormat = d;
    }
    inline void set_enable_gui(bool op) {
        m_settings.enableUI = op;
    }
    inline std::vector<Core::RenderPass*> get_render_passes() const {
        return m_renderpasses;
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
     Init renderpasses and
     */
    virtual void create_renderpasses();
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
    Resource like samplers, base textures and misc creation
    */
    virtual void init_resources();
    /*
    Clean all resources used
    */
    virtual void clean_resources();
    /*
    Link images of previous passes to current pass
    */
    void connect_renderpass(Core::RenderPass* const currentPass);
    /*
    Clean and recreates swapchain and framebuffers in the renderer. Useful to use
    when resizing context
    */
    void update_renderpasses();
    /*
    Initialize gui layout in case ther's one enabled
    */
    void init_gui();
#pragma endregion
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif // VK_RENDERER