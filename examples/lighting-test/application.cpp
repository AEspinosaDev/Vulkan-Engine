#include "application.h"
#include <filesystem>

void Application::init(Systems::RendererSettings settings) {
    m_window = new WindowGLFW("Lighting Test", 1280, 1024);

    m_window->init();
    m_window->set_window_icon(EXAMPLES_RESOURCES_PATH "textures/ico.png");

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

    m_renderer = new Systems::DeferredRenderer(m_window, ShadowResolution::VERY_LOW, settings);

    setup();

    setup_gui();
}

void Application::run(int argc, char* argv[]) {

    Systems::RendererSettings settings{};
    settings.samplesMSAA = MSAASamples::x1;
    settings.softwareAA  = true;
    settings.clearColor  = Vec4(0.02, 0.02, 0.02, 1.0);
    settings.enableUI    = true;

    if (argc == 1)
        std::cout << "No arguments submitted, initializing with default parameters..." << std::endl;

    for (int i = 0; i < argc; ++i)
    {
        std::string token(argv[i]);
        if (token == "-type")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "\"-type\" argument expects a rendering type keyword:" << std::endl;
                std::cerr << "[forward]" << std::endl;
                std::cerr << "[deferred]" << std::endl;
                return;
            }
            std::string type(argv[i + 1]);

            if (type == "forward")
            {
                // settings.renderingType = RendererType::TFORWARD;
                i++;
                continue;
            }
            if (type == "deferred")
            {
                // settings.renderingType = RendererType::TDEFERRED;
                i++;
                continue;
            }

            std::cerr << "\"--type\" invalid argument:" << std::endl;
            std::cerr << "[forward]" << std::endl;
            std::cerr << "[deferred]" << std::endl;
            return;
        } else if (token == "-aa")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "\"-aa\" argument expects an antialiasing type keyword:" << std::endl;
                std::cerr << "[none]" << std::endl;
                std::cerr << "[msaa4]" << std::endl;
                std::cerr << "[msaa8]" << std::endl;
                std::cerr << "[fxaa]" << std::endl;
                return;
            }
            std::string aaType(argv[i + 1]);
            if (aaType == "none")
                settings.samplesMSAA = MSAASamples::x1;
            if (aaType == "msaa4")
                settings.samplesMSAA = MSAASamples::x4;
            if (aaType == "msaa8")
                settings.samplesMSAA = MSAASamples::x8;
            // if (aaType == "fxaa")
            //     settings.AAtype = MSAASamples::FXAA;

            i++;
            continue;
        } else if (token == "-gui")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "\"-gui\" argument expects an enabling gui type keyword:" << std::endl;
                std::cerr << "[false]" << std::endl;
                std::cerr << "[true]" << std::endl;
                return;
            }
            std::string enableGui(argv[i + 1]);
            if (enableGui == "true")
                settings.enableUI = true;
            if (enableGui == "false")
                settings.enableUI = false;
            i++;
            continue;
        }
        continue;
    }

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
    camera->set_position(Vec3(0.0f, 0.0f, -5.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);
    camera->enable_frustrum_culling(false);

    m_scene = new Scene(camera);

    m_scene->enable_fog(false);

    int   gridSize = 10;   // 10x10 grid
    float spacing  = 4.0f; // Spacing between lights

    float offset = (gridSize - 1) * spacing / 2.0f;

    for (int i = 0; i < gridSize; ++i)
    {
        for (int j = 0; j < gridSize; ++j)
        {
            PointLight* light = new PointLight();

            // Calculate the position for the light, centered around 0.0
            float x = i * spacing - offset;
            float y = 6.5f; // Fixed height for all lights
            float z = j * spacing - offset;

            // Set the position of the light
            light->set_position({x, y, z});
            light->set_intensity(4.0);

            // Set the light to not cast shadows
            light->set_cast_shadows(false);

            // Generate a random color
            glm::vec3 color(static_cast<float>(rand()) / RAND_MAX,
                            static_cast<float>(rand()) / RAND_MAX,
                            static_cast<float>(rand()) / RAND_MAX);

            // Set the color of the light
            light->set_color(color);

            // Add the light to the scene
            m_scene->add(light);
        }
    }

    Mesh* terrainMesh = new Mesh();
    terrainMesh->push_geometry(Geometry::create_quad());
    terrainMesh->set_scale(25.0);
    terrainMesh->set_position({0.0, -2.0, 0.0});
    terrainMesh->set_rotation({-90.0, 0.0, 0.0});
    Texture* floorText = new Texture();
    Tools::Loaders::load_texture(floorText, TEXTURE_PATH + "floor_diffuse.jpg");
    Texture* floorNormalText = new Texture();
    Tools::Loaders::load_texture(floorNormalText, TEXTURE_PATH + "floor_normal.jpg");
    Texture* floorRoughText = new Texture();
    Tools::Loaders::load_texture(floorRoughText, TEXTURE_PATH + "floor_roughness.jpg");
    auto terrainMat = new PhysicalMaterial();
    terrainMat->set_albedo({0.43f, 0.28f, 0.23f});
    terrainMat->set_albedo_texture(floorText);
    terrainMat->set_roughness_texture(floorRoughText);
    terrainMat->set_tile({25.0f, 25.0f});
    terrainMesh->push_material(terrainMat);
    terrainMesh->set_name("Terrain");
    m_scene->add(terrainMesh);

    Mesh* kabutoMesh = new Mesh();
    Tools::Loaders::load_3D_file(kabutoMesh, EXAMPLES_RESOURCES_PATH "meshes/kabuto.obj", false);
    kabutoMesh->set_rotation(glm::vec3(0.0, 180, 0.0));
    kabutoMesh->set_position({-5.0, 0.0, 5.0});
    auto     kabutoMat  = new PhysicalMaterial();
    Texture* kabutoText = new Texture();
    Tools::Loaders::load_texture(kabutoText, TEXTURE_PATH + "kabuto_color.png");
    kabutoMat->set_albedo_texture(kabutoText);
    kabutoMat->set_albedo({0.0, 1.0, 0.0});
    kabutoMat->set_metalness(0.8f);
    kabutoMat->set_roughness(0.4f);
    kabutoMesh->push_material(kabutoMat);
    kabutoMesh->set_name("Kabuto");
    m_scene->add(kabutoMesh);

    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 6; ++j)
        {
            Mesh* kabutoMesh2 = kabutoMesh->clone();
            // kabutoMesh2->set_position({5.0, 0.0, 5.0});

            // Calculate the position for the light, centered around 0.0
            float x = i * spacing - offset;
            float y = 0.0; // Fixed height for all lights
            float z = j * spacing - offset;

            // Set the position of the light
            kabutoMesh2->set_position({x, y, z});

            m_scene->add(kabutoMesh2);

            m_scene->add(kabutoMesh2);
        }
    }

    m_scene->set_ambient_color({0.2, 0.25, 0.61});

    m_controller = new Tools::Controller(camera, m_window);
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

    m_interface.object->set_object(m_interface.scene->get_selected_object());
}

void Application::tick() {
    float currentTime      = (float)m_window->get_time_elapsed();
    m_time.delta           = currentTime - m_time.last;
    m_time.last            = currentTime;
    m_time.framesPerSecond = 1.0f / m_time.delta;

    update();

    m_renderer->render(m_scene);
}
