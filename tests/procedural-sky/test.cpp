#include "test.h"
#include <filesystem>

void Application::init( Systems::RendererSettings settings ) {
    m_window = std::make_shared<WindowGLFW>( "Procedural Sky Test", 1280, 1024 );

    m_window->init();
    m_window->set_window_icon( TESTS_RESOURCES_PATH "textures/test.png" );

    m_window->set_window_size_callback(
        std::bind( &Application::window_resize_callback, this, std::placeholders::_1, std::placeholders::_2 ) );
    m_window->set_mouse_callback(
        std::bind( &Application::mouse_callback, this, std::placeholders::_1, std::placeholders::_2 ) );
    m_window->set_key_callback( std::bind( &Application::keyboard_callback,
                                           this,
                                           std::placeholders::_1,
                                           std::placeholders::_2,
                                           std::placeholders::_3,
                                           std::placeholders::_4 ) );

    m_renderer = std::make_shared<Systems::DeferredRenderer>( m_window, settings );

    setup();
    m_renderer->init();
    setup_gui();
}

void Application::run( int argc, char* argv[] ) {

    Systems::RendererSettings settings {};
    settings.bufferingType    = BufferingType::DOUBLE;
    settings.samplesMSAA      = MSAASamples::x1;
    settings.clearColor       = Vec4( 0.02, 0.02, 0.02, 1.0 );
    settings.enableUI         = true;
    settings.enableRaytracing = true;
    settings.softwareAA       = SoftwareAA::FXAA;

    init( settings );
    while ( !m_window->get_window_should_close() )
    {

        // I-O
        m_window->poll_events();

        tick();
    }
    m_renderer->shutdown( m_scene );
}

void Application::setup() {
    const std::string SCENE_PATH( TESTS_RESOURCES_PATH "scenes/" );

    m_scene = new Scene();
    Tools::SceneLoader loader( true );
    loader.load_scene( m_scene, SCENE_PATH + "sky.xml" );

    m_scene->set_ambient_intensity( 0.1f );
    m_scene->use_IBL( false );

    m_camera     = m_scene->get_active_camera();
    m_controller = new Tools::Controller( m_camera, m_window.get(), ControllerMovementType::WASD );
}

void Application::setup_gui() {
    m_interface.overlay = new Tools::GUIOverlay(
        (float)m_window->get_extent().width, (float)m_window->get_extent().height, GuiColorProfileType::DARK );

    Tools::Panel* explorerPanel = new Tools::Panel( "EXPLORER", 0, 0, 0.2f, 0.7f, PanelWidgetFlags::NoMove, false );
    m_interface.scene           = new Tools::ExplorerWidget( m_scene, m_renderer.get() );
    explorerPanel->add_child( m_interface.scene );
    explorerPanel->add_child( new Tools::Space() );
    explorerPanel->add_child( new Tools::ControllerWidget( m_controller ) );
    explorerPanel->add_child( new Tools::Separator() );
    explorerPanel->add_child( new Tools::TextLine( " Application average" ) );
    explorerPanel->add_child( new Tools::Profiler() );
    explorerPanel->add_child( new Tools::Space() );

    m_interface.overlay->add_panel( explorerPanel );
    m_interface.explorer = explorerPanel;

    Tools::Panel* propertiesPanel =
        new Tools::Panel( "OBJECT PROPERTIES", 0.75f, 0, 0.25f, 0.8f, PanelWidgetFlags::NoMove, true );
    m_interface.object = new Tools::ObjectExplorerWidget();
    propertiesPanel->add_child( m_interface.object );

    m_interface.overlay->add_panel( propertiesPanel );
    m_interface.properties = propertiesPanel;
}

void Application::update() {
    if ( !m_interface.overlay->wants_to_handle_input() )
        m_controller->handle_keyboard( 0, 0, m_time.delta );

    m_interface.object->set_object( m_interface.scene->get_selected_object() );
}

void Application::tick() {
    float currentTime      = (float)m_window->get_time_elapsed();
    m_time.delta           = currentTime - m_time.last;
    m_time.last            = currentTime;
    m_time.framesPerSecond = 1.0f / m_time.delta;

    update();

    m_interface.overlay->render();
    m_renderer->render( m_scene );
}
