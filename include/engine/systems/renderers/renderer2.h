/*
        This file is part of Vulkan-Engine, a simple to use Vulkan based 3D
   library

        MIT License

        Copyright (c) 2023 Antonio Espinosa Garcia

        ////////////////////////////////////////////////////////////////////////////////////

*/
#ifndef RENDERER2_H
#define RENDERER2_H

#include <engine/common.h>

#include <engine/render/render_graph.h>
#include <engine/render/render_resources.h>
#include <engine/render/render_view_builder.h>

#include <engine/core/windows/window.h>

#include <engine/tools/loaders.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {

/**
 * Basic class. Renders a given scene data to a given window. Fully
 * parametrizable. It has to be inherited for achieving a higher end
 * application.
 */
class BaseRenderer2
{
#pragma region Properties
protected:
    /*Main properties*/
    ptr<Graphics::Device> m_device;
    ptr<Core::IWindow>    m_window;
    Extent2D              m_headlessExtent {}; // In case is headless

    /*Render Data*/
    Render::RenderGraph        m_graph;
    std::vector<Render::Frame> m_frames; // Per frame resources
    Render::Resources          m_shared; // Shared resources
    Render::RenderViewBuilder  m_viewBuilder;

    /* Settings */
    Render::Settings m_settings {};

    /*Automatic deletion queue*/
    Utils::DeletionQueue m_deletionQueue;

    /*Query*/
    uint32_t m_currentFrame       = 0;
    bool     m_initialized        = false;
    bool     m_headless           = false;
    bool     m_updateFramebuffers = false;

#pragma endregion
public:
    BaseRenderer2( const ptr<Core::IWindow>& window )
        : m_window( window )
        , m_device( nullptr ) {
        if ( !window )
            m_headless = true;
    }
    BaseRenderer2( const ptr<Core::IWindow>& window, Render::Settings settings )
        : m_window( window )
        , m_settings( settings )
        , m_device( nullptr ) {
        if ( !window )
            m_headless = true;
    }
    // Headless rendering
    BaseRenderer2( Extent2D displayExtent )
        : m_window( nullptr )
        , m_device( nullptr )
        , m_headlessExtent { displayExtent } {
        m_headless = true;
    }

#pragma region Getters & Setters

    inline ptr<Core::IWindow> const get_window() const {
        return m_window;
    }
    inline bool headless() const {
        return m_headless;
    }
    inline Render::Settings get_settings() const {
        return m_settings;
    }
    virtual inline void set_settings( Render::Settings settings ) {
        if ( m_settings.screenSync != settings.screenSync )
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
    void run( Core::Scene* const scene );
    /**
     * Renders a given scene.
     */
    void render( Core::Scene* const scene );
    /**
     * Shut the renderer down.
     */
    void shutdown( Core::Scene* const scene );

#pragma endregion
#pragma region Core Functions
protected:
    /* Resource like samplers, base textures and misc creation */
    virtual void init_resources() = 0;
    /* Register shader programs used by the renderer */
    virtual void register_shaders() = 0;
    /* Create passes for the render graph */
    virtual void configure_passes() = 0;
    /* Update swapchain if context modified*/
    void update_swapchain( Extent2D extent );
    /*Clean all resources used  */
    virtual void clean_resources();
    /*Initialize gui layout in case ther's one enabled*/
    void init_gui();

#pragma endregion
#pragma region Callbacks
    /* What to do when initiating the renderer */
    virtual void on_init() {}
    /* What to do just before rendering */
    virtual void on_before_render( Core::Scene* const scene );
    /* What to do just before rendering*/
    virtual void on_after_render( RenderResult& renderResult, Core::Scene* const scene );
    /* What to do when shutting down the renderer */
    virtual void on_shutdown( Core::Scene* const scene ) {}
#pragma endregion
};
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END

#endif // VK_RENDERER