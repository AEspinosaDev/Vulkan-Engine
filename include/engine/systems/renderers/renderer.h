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
#include <engine/core/passes/pass.h>
#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/*
Renderer Global Settings Data
*/
struct RendererSettings {

    MSAASamples      samplesMSAA           = MSAASamples::x4;          // Multisampled AA (when possible)
    BufferingType    bufferingType         = BufferingType::DOUBLE;    // Buffering type (Usual: double buffering)
    SyncType         screenSync            = SyncType::MAILBOX;        // Type of display synchronization
    ColorFormatType  displayColorFormat    = SRGBA_8;                  // Color format used for presentation
    FloatPrecission  highDynamicPrecission = FloatPrecission::F16;     // HDR operations floating point precission
    FloatPrecission  depthPrecission       = FloatPrecission::F32;     // Depth operations floating point precission
    Vec4             clearColor            = Vec4{0.0, 0.0, 0.0, 1.0}; // Clear color of visible color buffer
    SoftwareAA       softwareAA            = SoftwareAA::NONE;
    ShadowResolution shadowQuality         = ShadowResolution::MEDIUM;
    bool             autoClearColor        = true;
    bool             autoClearDepth        = true;
    bool             autoClearStencil      = true;
    bool             enableUI              = false;
    bool             enableRaytracing      = true;
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
    /*Main properties*/
    Graphics::Device*            m_device;
    std::vector<Graphics::Frame> m_frames;
    Core::IWindow*               m_window;
    Extent2D                     m_headlessExtent{};

    /*Settings*/
    RendererSettings m_settings{};

    /*Passes & Attachments*/
    std::vector<Core::BasePass*> m_passes;
    std::vector<Graphics::Image> m_attachments;

    /*Automatic deletion queue*/
    Utils::DeletionQueue m_deletionQueue;

    /*Query*/
    uint32_t m_currentFrame       = 0;
    bool     m_initialized        = false;
    bool     m_headless           = false;
    bool     m_updateFramebuffers = false;

#pragma endregion
  public:
    BaseRenderer(Core::IWindow* window)
        : m_window(window)
        , m_device(nullptr) {
        if (!window)
            m_headless = true;
    }
    BaseRenderer(Core::IWindow* window, RendererSettings settings)
        : m_window(window)
        , m_settings(settings)
        , m_device(nullptr) {
        if (!window)
            m_headless = true;
    }
    // Headless rendering
    BaseRenderer(Extent2D displayExtent)
        : m_window(nullptr)
        , m_device(nullptr)
        , m_headlessExtent{displayExtent} {
        m_headless = true;
    }

#pragma region Getters & Setters

    inline Core::IWindow* const get_window() const {
        return m_window;
    }
    inline bool headless() const {
        return m_headless;
    }

    inline RendererSettings get_settings() const {
        return m_settings;
    }
    virtual inline void set_settings(RendererSettings settings) {
        if (m_settings.screenSync != settings.screenSync)
            m_updateFramebuffers = true;

        m_settings = settings;
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
    /*
     * Capture one of the attachment images in a given frame as a CPU texture.
     */
    Core::ITexture* capture_texture(uint32_t attachmentId);

#pragma endregion
#pragma region Core Functions
  protected:
    /*
     Init passes
     */
    virtual void create_passes();
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
    Clean and recreates swapchain and framebuffers in the renderer. Useful to use
    when resizing context
    */
    void update_framebuffers(Extent2D extent);
    /*
    Initialize gui layout in case ther's one enabled
    */
    void init_gui();
#pragma endregion
#pragma region Utility

    template <typename T> inline T get_pass(uint32_t id) {
        T pass = id < m_passes.size() ? static_cast<T>(m_passes[id]) : nullptr;
        ASSERT_PTR(pass);
        return pass;
    }

#pragma endregion
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif // VK_RENDERER