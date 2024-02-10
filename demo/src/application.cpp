#include <demo/application.h>
#include <filesystem>

void VulkanRenderer::init()
{
    m_window = new Window("VK Engine", 1280, 1024);

    m_window->init();

    m_window->set_window_size_callback(std::bind(&VulkanRenderer::window_resize_callback, this, std::placeholders::_1,
                                                 std::placeholders::_2));
    m_window->set_mouse_callback(std::bind(&VulkanRenderer::mouse_callback, this, std::placeholders::_1,
                                           std::placeholders::_2));
    m_window->set_key_callback(std::bind(&VulkanRenderer::keyboard_callback, this, std::placeholders::_1,
                                         std::placeholders::_2, std::placeholders::_3,
                                         std::placeholders::_4));

    RendererSettings settings{};
    settings.AAtype = AntialiasingType::MSAA_x8;
    settings.clearColor = glm::vec4(0.02, 0.02, 0.02, 1.0);
    settings.enableUI = true;

    m_renderer = new Renderer(m_window, settings);

    setup();

    setup_gui();
}

void VulkanRenderer::run()
{
    init();
    while (!m_window->get_window_should_close())
    {

        // I-O
        Window::poll_events();

        tick();
    }
    m_renderer->shutdown();
}

void VulkanRenderer::setup()
{
    std::string meshDir(MODEL_DIR);
    std::string textDir(TEXTURE_DIR);

    std::string engineMeshDir(VK_MODEL_DIR);

    camera = new Camera();
    camera->set_position(glm::vec3(0.0f, 0.0f, -5.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    m_scene = new Scene(camera);

    m_scene->add(new PointLight());
    m_scene->get_light()->set_position({-3.0f, 3.0f, 0.0f});

    Mesh *toriiMesh = new Mesh();
    auto toriiMat = new PhysicallyBasedMaterial();
    Texture *toriiT = new Texture();
    toriiT->load_image(textDir + "torii_color.png");
    Texture *toriiN = new Texture();
    toriiN->load_image(textDir + "torii_normal.png");
    Texture *toriiM = new Texture();
    toriiM->load_image(textDir + "torii_mask.png");
    toriiMat->set_albedo_texture(toriiT);
    toriiMat->set_normal_texture(toriiN);
    toriiMat->set_mask_texture(toriiM, UNREAL_ENGINE);
    toriiMesh->set_material(toriiMat);
    toriiMesh->load_file(meshDir + "torii.obj",true);
    toriiMesh->set_name("Torii");
    toriiMesh->set_scale(0.2f);
    toriiMesh->set_position({1.6, -2.3, 6.1});
    toriiMesh->set_rotation({0.0, 28.0f, 8.0f});
    m_scene->add(toriiMesh);

    Mesh *terrainMesh = new Mesh();
    terrainMesh->set_scale(10.0);
    terrainMesh->set_position({0.0, -4.0, 0.0});
    terrainMesh->load_file(meshDir + "terrain.obj",true);
    Texture *floorText = new Texture();
    floorText->load_image(textDir + "floor_diffuse.jpg");
    Texture *floorNormalText = new Texture();
    floorNormalText->load_image(textDir + "floor_normal.jpg");
    Texture *floorRoughText = new Texture();
    floorRoughText->load_image(textDir + "floor_roughness.jpg");
    auto terrainMat = new PhysicallyBasedMaterial();
    terrainMat->set_albedo({0.43f, 0.28f, 0.23f, 1.0});
    terrainMat->set_albedo_texture(floorText);
    // mat->set_normal_texture(floorNormalText);
    terrainMat->set_roughness_texture(floorRoughText);
    terrainMat->set_tile({25.0f, 25.0f});
    terrainMesh->set_material(terrainMat);
    terrainMesh->set_name("Terrain");
    m_scene->add(terrainMesh);

    Mesh *boxMesh = new Mesh();
    boxMesh->set_position({-3, -2.5, 5.0});
    boxMesh->set_rotation({0.0, 20.0f, 0.0f});
    boxMesh->load_file(engineMeshDir + "cube.obj");
    Texture *woodText = new Texture();
    woodText->load_image(textDir + "wood_diffuse.jpg");
    auto boxMat = new PhysicallyBasedMaterial();
    boxMat->set_albedo_texture(woodText);
    boxMesh->set_material(boxMat);
    boxMesh->set_name("Box");
    m_scene->add(boxMesh);

    auto lightMat = new UnlitMaterial();
    lightMat->set_color(glm::vec4(m_scene->get_light()->get_color(), 1.0f));
    m_lightDummy = new Mesh();
    m_lightDummy->load_file(engineMeshDir + "sphere.obj");
    m_lightDummy->set_material(lightMat);
    m_lightDummy->set_scale(0.5f);
    m_lightDummy->set_name("Light Gizmo");
    m_scene->add(m_lightDummy);

    Mesh *kabutoMesh = new Mesh();
    kabutoMesh->load_file(meshDir + "kabuto.obj");
    kabutoMesh->set_rotation(glm::vec3(0.0, 180, 0.0));
    auto kabutoMat = new PhysicallyBasedMaterial();
    Texture *kabutoText = new Texture();
    kabutoText->load_image(textDir + "kabuto_color.png");
    kabutoMat->set_albedo_texture(kabutoText);
    kabutoMat->set_albedo({0.0, 1.0, 0.0, 1.0});
    kabutoMat->set_metalness(0.8f);
    kabutoMat->set_roughness(0.4f);
    kabutoMesh->set_material(kabutoMat);
    kabutoMesh->set_name("Kabuto");
    m_scene->add(kabutoMesh);

    // Space scene

    // Mesh *earth = new Mesh();
    // earth->load_file(meshDir + "temple.obj");
    // // earth->set_rotation(glm::vec3(0.0, 3.14, 0.0));
    // auto earthMat = new PhysicallyBasedMaterial();
    // Texture *earthText = new Texture();
    // // earthText->load_image(textDir + "Earth_ALB.png");
    // // earthMat->set_albedo_texture(earthText);
    // earthMat->set_albedo({0.0, 1.0, 0.0, 1.0});
    // earthMat->set_metalness(0.8f);
    // earthMat->set_roughness(0.4f);
    // earth->set_material(earthMat);
    // earth->set_name("Earth");
    // m_scene->add(earth);

    m_controller = new Controller(camera);
}

void VulkanRenderer::setup_gui()
{
    m_interface.overlay = new GUIOverlay((float)m_window->get_extent()->width, (float)m_window->get_extent()->height, GuiColorProfileType::DARK);

    Panel *tutorialPanel = new Panel("TUTORIAL", 0, 0.8f, 0.2f, 0.2f, PanelWidgetFlags::NoMove, false, true);

    tutorialPanel->add_child(new Space());
    tutorialPanel->add_child(new Separator("CONTROLS"));
    tutorialPanel->add_child(new Separator());
    tutorialPanel->add_child(new TextLine("WASD: move camera.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new TextLine("QE: camera down/up.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new TextLine("Mouse + Left: rotate camera.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new TextLine("L: toggle light animation", TextWidgetType::BULLET));
    tutorialPanel->add_child(new TextLine("F11: toggle fullscreen/windowed mode.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new TextLine("Esc: exit application.", TextWidgetType::BULLET));
    tutorialPanel->add_child(new Space());
    tutorialPanel->add_child(new Separator());
    tutorialPanel->add_child(new TextLine("Enjoy changing the parameters!"));

    m_interface.overlay->add_panel(tutorialPanel);
    m_interface.tutorial = tutorialPanel;

    Panel *explorerPanel = new Panel("EXPLORER", 0, 0, 0.2f, 0.7f, PanelWidgetFlags::NoMove, false);
    m_interface.scene = new SceneExplorerWidget(m_scene);
    explorerPanel->add_child(m_interface.scene);
    explorerPanel->add_child(new Space());
    explorerPanel->add_child(new RendererSettingsWidget(m_renderer));
    explorerPanel->add_child(new Separator());
    explorerPanel->add_child(new TextLine(" Application average (No cap)"));
    explorerPanel->add_child(new Profiler());
    explorerPanel->add_child(new Space());

    m_interface.overlay->add_panel(explorerPanel);
    m_interface.explorer = explorerPanel;

    Panel *propertiesPanel = new Panel("OBJECT PROPERTIES", 0.75f, 0, 0.25f, 0.8f, PanelWidgetFlags::NoMove, true);
    m_interface.object = new ObjectExplorerWidget();
    propertiesPanel->add_child(m_interface.object);

    m_interface.overlay->add_panel(propertiesPanel);
    m_interface.properties = propertiesPanel;

    m_renderer->set_gui_overlay(m_interface.overlay);
}

void VulkanRenderer::update()
{
    if (!m_interface.overlay->wants_to_handle_input())
        m_controller->handle_keyboard(m_window->get_window_obj(), 0, 0, m_time.delta);

    // Rotate the vector around the ZX plane
    auto light = m_scene->get_light();
    if (animateLight)
    {
        float rotationAngle = glm::radians(10.0f * m_time.delta);
        float _x = light->get_position().x * cos(rotationAngle) - light->get_position().z * sin(rotationAngle);
        float _z = light->get_position().x * sin(rotationAngle) + light->get_position().z * cos(rotationAngle);

        light->set_position({_x, light->get_position().y, _z});
    }
    m_lightDummy->set_position(light->get_position());
    dynamic_cast<UnlitMaterial *>(m_lightDummy->get_material())->set_color(glm::vec4(light->get_color(), 1.0f));

    m_interface.object->set_object(m_interface.scene->get_selected_object());
}

void VulkanRenderer::tick()
{
    float currentTime = (float)Window::get_time_elapsed();
    m_time.delta = currentTime - m_time.last;
    m_time.last = currentTime;
    m_time.framesPerSecond = 1.0f / m_time.delta;

    update();

    m_renderer->render(m_scene);
}
