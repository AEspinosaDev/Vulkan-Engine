/*
        This file is part of Vulkan-Engine, a simple to use Vulkan based 3D
   library

        MIT License

        Copyright (c) 2023 Antonio Espinosa Garcia

*/
#include <engine/systems/renderers/renderer2.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Systems {
void BaseRenderer2::init() {

    // Init Vulkan Device
    m_device = std::make_shared<Graphics::Device>();

    if ( !m_headless )
    {
        if ( !m_window->initialized() )
            m_window->init();
        void* windowHandle { nullptr };
        m_window->get_handle( windowHandle );
        m_device->init( windowHandle,
                        m_window->get_windowing_system(),
                        m_window->get_extent(),
                        static_cast<uint32_t>( m_settings.bufferingType ),
                        m_settings.displayColorFormat,
                        m_settings.screenSync );
    } else
    {
        m_device->init_headless();
    }

    init_resources();
    register_shaders();

    if ( m_settings.enableUI && !m_headless )
        init_gui();

    m_initialized = true;
}
void BaseRenderer2::run( Core::Scene* const scene ) {
    ASSERT_PTR( m_window );
    while ( !m_window->get_window_should_close() )
    {
        // I-O
        PROFILING_FRAME();
        m_window->poll_events();
        render( scene );
    }
    shutdown( scene );
}

void BaseRenderer2::shutdown( Core::Scene* const scene ) {
    m_device->wait_idle();

    on_shutdown( scene );

    if ( m_initialized )
    {
        m_deletionQueue.flush();

        clean_resources();
        Render::Resources::clean_scene( scene );

        if ( m_settings.enableUI && !m_headless )
            m_device->destroy_imgui();

        m_device->cleanup();
    }

    if ( !m_headless )
        m_window->destroy();

    glfwTerminate();
}

void BaseRenderer2::on_before_render( Core::Scene* const scene ) {
}

void BaseRenderer2::on_after_render( RenderResult& renderResult, Core::Scene* const scene ) {
    PROFILING_EVENT()

    // Save old camera state
    auto camera           = scene->get_active_camera();
    m_shared.prevViewProj = camera->get_projection() * camera->get_view();

    if ( !m_headless )
    {
        if ( renderResult == RenderResult::ERROR_OUT_OF_DATE_KHR || renderResult == RenderResult::SUBOPTIMAL_KHR || m_window->is_resized() ||
             m_updateFramebuffers )
        {
            m_window->set_resized( false );
            m_window->update_framebuffer();
            update_swapchain( m_window->get_extent() );
            scene->get_active_camera()->set_projection( m_window->get_extent().width, m_window->get_extent().height );
        } else if ( renderResult != RenderResult::SUCCESS )
        { throw VKFW_Exception( "failed to present swap chain image!" ); }
    }
    m_currentFrame = ( m_currentFrame + 1 ) % m_frames.size();
}

void BaseRenderer2::render( Core::Scene* const scene ) {
    PROFILING_FRAME();
    PROFILING_EVENT()

    if ( !m_initialized )
        init();

    if ( !scene->get_active_camera() )
        return;

    uint32_t imageIndex = 0;

    m_graph.begin_frame( m_frames[m_currentFrame] );

    if ( !m_headless )
    {
        RenderResult result = m_device->aquire_present_image( m_frames[m_currentFrame].get_present_semaphore(), imageIndex );

        if ( result == RenderResult::ERROR_OUT_OF_DATE_KHR )
        {
            m_window->update_framebuffer();
            update_swapchain( m_window->get_extent() );
            return;
        } else if ( result != RenderResult::SUCCESS && result != RenderResult::SUBOPTIMAL_KHR )
        {
            throw VKFW_Exception( "failed to acquire swap chain image!" );
        }
    }

    on_before_render( scene );

    const Extent2D DISPLAY_EXTENT = !m_headless ? m_window->get_extent() : m_headlessExtent;

    Render::RenderView renderView = m_viewBuilder.build( m_device, &m_frames[m_currentFrame], scene, DISPLAY_EXTENT, m_settings );
    renderView.presentImageIndex  = imageIndex;

    configure_passes();

    m_graph.end_frame( renderView, m_shared );

    RenderResult renderResult = RenderResult::SUCCESS;
    if ( !m_headless )
        m_device->present_image( m_frames[m_currentFrame].get_render_semaphore(), imageIndex );

    on_after_render( renderResult, scene );
}

void BaseRenderer2::update_swapchain( Extent2D extent ) {

    m_device->wait_idle();
    m_device->update_swapchain( extent, static_cast<uint32_t>( m_settings.bufferingType ), m_settings.displayColorFormat, m_settings.screenSync );

    m_updateFramebuffers = false;
}

void BaseRenderer2::init_gui() {

    // if ( m_settings.enableUI )
    // {
    //     // Look for default pass
    //     ptr<Render::BasePass> defaultPass = nullptr;
    //     for ( auto& pass : m_passes )
    //     {
    //         if ( pass->is_active() && pass->default_pass() )
    //         {
    //             defaultPass = pass;
    //         }
    //     };

    //     void* windowHandle;
    //     m_window->get_handle( windowHandle );
    //     m_device->init_imgui( windowHandle,
    //                           m_window->get_windowing_system(),
    //                           std::static_pointer_cast<Render::BaseGraphicPass>( defaultPass )->get_renderpass(),
    //                           std::static_pointer_cast<Render::BaseGraphicPass>( defaultPass )->get_renderpass().targetInfos[0].imageConfig.samples );
    // }
}



void BaseRenderer2::clean_resources() {
    for ( size_t i = 0; i < m_frames.size(); i++ )
        m_frames[i].cleanup();

    m_shared.clean_shared_resources();
}

} // namespace Systems

VULKAN_ENGINE_NAMESPACE_END