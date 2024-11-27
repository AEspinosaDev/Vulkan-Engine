#include "application.h"
#include <filesystem>

void Application::init(Systems::RendererSettings settings) {
    m_window = new WindowGLFW("Raytracing Example", 1280, 1024);

    m_window->init();

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

    m_renderer = new Systems::ForwardRenderer(m_window, true, ShadowResolution::LOW, settings);

    setup();
    setup_gui();
}

void Application::run(int argc, char* argv[]) {

    Systems::RendererSettings settings{};
    settings.samplesMSAA      = MSAASamples::x1;
    settings.clearColor       = Vec4(0.02, 0.02, 0.02, 1.0);
    settings.enableUI         = true;
    settings.enableRaytracing = true;

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
    const std::string MESH_PATH(EXAMPLES_RESOURCES_PATH "meshes/");
    const std::string TEXTURE_PATH(EXAMPLES_RESOURCES_PATH "textures/");
    const std::string ENGINE_MESH_PATH(ENGINE_RESOURCES_PATH "meshes/");

    camera = new Camera();
    camera->set_position(Vec3(0.0f, 0.0f, -7.2f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    m_scene = new Scene(camera);

    Mesh* lightDummy = new Mesh();
    Tools::Loaders::load_3D_file(lightDummy, ENGINE_MESH_PATH + "sphere.obj", false);
    lightDummy->push_material(new UnlitMaterial());
    lightDummy->set_scale(0.5f);
    lightDummy->ray_hittable(false);
    lightDummy->cast_shadows(false);
    lightDummy->set_name("Gizmo");

    PointLight* light = new PointLight();
    light->set_position({-3.0f, 3.0f, 0.0f});
    light->set_area_of_effect(20.0f);
    light->set_shadow_fov(115.0f);
    light->set_shadow_type(ShadowType::RAYTRACED_SHADOW);
    light->set_area(0.5f);
    // light->add_child(lightDummy);
    light->set_name("Light");

    m_scene->add(light);

    Mesh* toriiMesh = new Mesh();
    auto  toriiMat  = new PhysicallyBasedMaterial();

    Texture* toriiT = new Texture();
    Tools::Loaders::load_texture(toriiT, TEXTURE_PATH + "torii_color.png");
    toriiMat->set_albedo_texture(toriiT);

    Texture* toriiN = new Texture();
    Tools::Loaders::load_texture(toriiN, TEXTURE_PATH + "torii_normal.png", TEXTURE_FORMAT_TYPE_NORMAL);
    toriiMat->set_normal_texture(toriiN);

    Texture* toriiM = new Texture();
    // Tools::Loaders::load_texture(toriiM, TEXTURE_PATH + "torii_mask.png");
    // toriiMat->set_mask_texture(toriiM, UNREAL_ENGINE);
    toriiMat->set_metalness(0.05);
    toriiMat->set_roughness(0.5);
    toriiMesh->push_material(toriiMat);

    Tools::Loaders::load_3D_file(toriiMesh, MESH_PATH + "torii.obj", false);
    toriiMesh->set_name("Torii");
    toriiMesh->set_scale(0.2f);
    toriiMesh->set_position({0.0, -2.3, 0.0});
    toriiMesh->set_rotation({0.0, 28.0f, 0.0f});
    m_scene->add(toriiMesh);

    Mesh* plane = new Mesh();
    plane->push_geometry(Geometry::create_quad());
    auto     terrainMat = new PhysicallyBasedMaterial();
    Texture* floorText  = new Texture();
    Tools::Loaders::load_texture(floorText, TEXTURE_PATH + "floor_diffuse.jpg");
    Texture* floorNormalText = new Texture();
    Tools::Loaders::load_texture(floorNormalText, TEXTURE_PATH + "floor_normal.jpg", TEXTURE_FORMAT_TYPE_NORMAL);
    Texture* floorRoughText = new Texture();
    Tools::Loaders::load_texture(floorRoughText, TEXTURE_PATH + "floor_roughness.jpg");
    terrainMat->set_albedo({0.43f, 0.28f, 0.23f});
    terrainMat->set_albedo_texture(floorText);
    terrainMat->set_normal_texture(floorNormalText);
    terrainMat->set_roughness_texture(floorRoughText);
    terrainMat->set_tile({3.0f, 3.0f});
    plane->push_material(terrainMat);
    plane->set_name("Floor");
    plane->set_position({0.0, -2.3, 0.0});
    plane->set_rotation({-90.0f, 0.0f, 0.0f});
    plane->set_scale(5.0f);

    Mesh* stoneMesh = new Mesh();
    Tools::Loaders::load_3D_file(stoneMesh, MESH_PATH + "stone_lantern.obj", false);
    auto     stoneMat      = new PhysicallyBasedMaterial();
    Texture* stonelanternT = new Texture();
    Tools::Loaders::load_texture(stonelanternT, TEXTURE_PATH + "stone_diffuse.png");
    stoneMat->set_albedo_texture(stonelanternT);
    Texture* stonelanternN = new Texture();
    Tools::Loaders::load_texture(stonelanternN, TEXTURE_PATH + "stone_normal.png", TEXTURE_FORMAT_TYPE_NORMAL);
    stoneMat->set_normal_texture(stonelanternN);
    stoneMesh->push_material(stoneMat);
    stoneMesh->set_name("Lantern");
    stoneMesh->set_position({2.0f, -2.3f, -2.3f});
    stoneMesh->set_rotation({0.0, 126.0f, 0.0f});
    stoneMesh->set_scale(1.5);
    stoneMat->set_roughness(0.5f);
    stoneMat->set_metalness(0.0f);
    m_scene->add(stoneMesh);

    m_scene->add(plane);

    m_scene->set_ambient_color({0.2, 0.25, 0.61});

    TextureHDR* envMap = new TextureHDR();
    Tools::Loaders::load_texture(envMap, TEXTURE_PATH + "cloudy.hdr");
    Skybox* sky = new Skybox(envMap);
    sky->set_color_intensity(0.25f);
    m_scene->set_skybox(sky);

    m_controller = new Tools::Controller(camera, m_window, ControllerMovementType::ORBITAL);
}

void Application::setup_gui() {
    m_interface.overlay = new Tools::GUIOverlay(
        (float)m_window->get_extent().width, (float)m_window->get_extent().height, GuiColorProfileType::DARK);

    Tools::Panel* tutorialPanel =
        new Tools::Panel("TUTORIAL", 0, 0.8f, 0.2f, 0.2f, PanelWidgetFlags::NoMove, false, true);

    tutorialPanel->add_child(new Tools::Space());
    tutorialPanel->add_child(new Tools::Separator("CONTROLS"));
    tutorialPanel->add_child(new Tools::Separator());
    tutorialPanel->add_child(new Tools::TextLine("WASD: move camera.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new Tools::TextLine("QE: camera down/up.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new Tools::TextLine("Mouse + Left: rotate camera.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new Tools::TextLine("L: toggle light animation", TextWidgetType::BULLET));
    tutorialPanel->add_child(new Tools::TextLine("F11: toggle fullscreen/windowed mode.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new Tools::TextLine("Esc: exit application.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new Tools::Space());
    tutorialPanel->add_child(new Tools::Separator());
    tutorialPanel->add_child(new Tools::TextLine("Enjoy changing the parameters!"));

    m_interface.overlay->add_panel(tutorialPanel);
    m_interface.tutorial = tutorialPanel;

    Tools::Panel* explorerPanel = new Tools::Panel("EXPLORER", 0, 0, 0.2f, 0.7f, PanelWidgetFlags::NoMove, false);
    m_interface.scene           = new Tools::SceneExplorerWidget(m_scene);
    explorerPanel->add_child(m_interface.scene);
    explorerPanel->add_child(new Tools::Space());
    explorerPanel->add_child(new Tools::RendererSettingsWidget(m_renderer));
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

    // m_renderer->set_gui_overlay(m_interface.overlay);
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
