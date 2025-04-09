#include "test.h"
#include <filesystem>

void Application::init(Systems::RendererSettings settings) {
    m_window = new WindowGLFW("Skin Test", 1280, 1024);

    m_window->init();
    m_window->set_window_icon(TESTS_RESOURCES_PATH "textures/test.png");

    m_window->set_window_size_callback(
        std::bind(&Application::window_resize_callback, this, std::placeholders::_1, std::placeholders::_2));
    m_window->set_mouse_callback(
        std::bind(&Application::mouse_callback, this, std::placeholders::_1, std::placeholders::_2));
    m_window->set_key_callback(std::bind(&Application::keyboard_callback,
                                         this,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3,
                                         std::placeholders::_4));

    Systems::DeferredRenderer* rndr = new Systems::DeferredRenderer(m_window, ShadowResolution::MEDIUM, settings);
    m_renderer                      = rndr;

    setup();
    m_renderer->init();
    setup_gui();
}

void Application::run(int argc, char* argv[]) {

    Systems::RendererSettings settings{};
    settings.bufferingType    = BufferingType::DOUBLE;
    settings.samplesMSAA      = MSAASamples::x1;
    settings.clearColor       = Vec4(0.02, 0.02, 0.02, 1.0);
    settings.enableUI         = true;
    settings.enableRaytracing = true;
    settings.softwareAA       = true;

    init(settings);
    while (!m_window->get_window_should_close())
    {

        // I-O
        m_window->poll_events();

        tick();
    }
    m_renderer->shutdown(m_scene);
}

void Application::setup() {
    const std::string SCENE_PATH(TESTS_RESOURCES_PATH "scenes/");
    const std::string MESH_PATH(TESTS_RESOURCES_PATH "meshes/");
    const std::string TEXTURE_PATH(TESTS_RESOURCES_PATH "textures/");

    auto camera = new Camera();
    camera->set_position(Vec3(0.0f, 0.5f, -1.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    m_scene = new Scene(camera);

    m_scene->add(new PointLight());
    m_scene->get_lights()[0]->set_position({-3.0f, 3.0f, 0.0f});
    m_scene->get_lights()[0]->set_shadow_fov(25.0f);
    m_scene->get_lights()[0]->set_intensity(1.0f);
    m_scene->get_lights()[0]->set_shadow_bias(0.001f);
    m_scene->get_lights()[0]->set_shadow_softness(5);

    Mesh*    headMesh     = new Mesh();
    auto     skinMaterial = new PhysicalMaterial();
    Texture* skinAlbedo   = new Texture();
    Tools::Loaders::load_texture(skinAlbedo, TEXTURE_PATH + "perry_albedo.png");
    skinMaterial->set_albedo_texture(skinAlbedo);
    Texture* skinNormal = new Texture();
    Tools::Loaders::load_texture(skinNormal, TEXTURE_PATH + "perry_normal.png", TEXTURE_FORMAT_UNORM);
    skinMaterial->set_normal_texture(skinNormal);
    skinMaterial->set_roughness(0.5);
    headMesh->push_material(skinMaterial);
    Tools::Loaders::load_3D_file(headMesh, MESH_PATH + "lee_perry.obj");
    headMesh->set_name("Head");
    headMesh->set_scale(2.0f);
    headMesh->set_rotation({0.0, 180.0f, 0.0f});
    m_scene->add(headMesh);

    m_scene->set_ambient_intensity(0.1f);
    m_scene->use_IBL(false);

    m_camera     = m_scene->get_active_camera();
    m_controller = new Tools::Controller(m_camera, m_window, ControllerMovementType::ORBITAL);
    m_controller->set_orbital_center({0.0, 0.5, 0.0});
}

void Application::setup_gui() {
    m_interface.overlay = new Tools::GUIOverlay(
        (float)m_window->get_extent().width, (float)m_window->get_extent().height, GuiColorProfileType::DARK);

    Tools::Panel* explorerPanel = new Tools::Panel("EXPLORER", 0, 0, 0.2f, 0.7f, PanelWidgetFlags::NoMove, false);
    m_interface.scene           = new Tools::ExplorerWidget(m_scene, m_renderer);
    explorerPanel->add_child(m_interface.scene);
    explorerPanel->add_child(new Tools::Space());
    explorerPanel->add_child(new Tools::ControllerWidget(m_controller));
    explorerPanel->add_child(new Tools::Separator());
    explorerPanel->add_child(new Tools::TextLine(" Application average"));
    explorerPanel->add_child(new Tools::Profiler());
    explorerPanel->add_child(new Tools::Space());

    m_interface.overlay->add_panel(explorerPanel);
    m_interface.explorer = explorerPanel;

    Tools::Panel* propertiesPanel =
        new Tools::Panel("OBJECT PROPERTIES", 0.75f, 0, 0.25f, 0.8f, PanelWidgetFlags::NoMove, true);
    m_interface.object = new Tools::ObjectExplorerWidget();
    propertiesPanel->add_child(m_interface.object);

    m_interface.overlay->add_panel(propertiesPanel);
    m_interface.properties = propertiesPanel;
}

void Application::update() {
    if (!m_interface.overlay->wants_to_handle_input())
        m_controller->handle_keyboard(0, 0, m_time.delta);

    // Rotate the vector around the ZX plane
    auto light = m_scene->get_lights()[0];
    if (animateLight)
    {
        float rotationAngle = glm::radians(10.0f * m_time.delta);
        float _x = light->get_position().x * cos(rotationAngle) - light->get_position().z * sin(rotationAngle);
        float _z = light->get_position().x * sin(rotationAngle) + light->get_position().z * cos(rotationAngle);

        light->set_position({_x, light->get_position().y, _z});
    }
    // static_cast<UnlitMaterial*>(static_cast<Mesh*>(light->get_children().front())->get_material(0))
    //     ->set_color({light->get_color() * light->get_intensity(), 1.0f});

    m_interface.object->set_object(m_interface.scene->get_selected_object());
}

void Application::tick() {
    float currentTime      = (float)m_window->get_time_elapsed();
    m_time.delta           = currentTime - m_time.last;
    m_time.last            = currentTime;
    m_time.framesPerSecond = 1.0f / m_time.delta;

    update();

    m_interface.overlay->render();
    m_renderer->render(m_scene);
}
