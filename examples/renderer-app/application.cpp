#include "application.h"
#include <filesystem>

void VulkanRenderer::init(Systems::RendererSettings settings, Systems::ForwardRendererSettings settings2) {
    m_window = new WindowGLFW("VK Engine", 1280, 1024);

    m_window->init();

    m_window->set_window_size_callback(
        std::bind(&VulkanRenderer::window_resize_callback, this, std::placeholders::_1, std::placeholders::_2));
    m_window->set_mouse_callback(
        std::bind(&VulkanRenderer::mouse_callback, this, std::placeholders::_1, std::placeholders::_2));
    m_window->set_key_callback(std::bind(&VulkanRenderer::keyboard_callback,
                                         this,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3,
                                         std::placeholders::_4));

    m_renderer = new Systems::ForwardRenderer(m_window, settings, settings2);

    setup();

    setup_gui();
}

void VulkanRenderer::run(int argc, char* argv[]) {

    Systems::RendererSettings settings{};
    settings.samplesMSAA = MSAASamples::MSAA_x8;
    // settings.colorFormat = ColorFormatType::SRGBA_8;
    // settings.depthFormat = DepthFormatType::D16F;
    settings.clearColor = Vec4(0.02, 0.02, 0.02, 1.0);
    settings.enableUI   = true;
    Systems::ForwardRendererSettings settings2{};
    settings2.shadowQuality = ShadowResolution::MEDIUM;
    settings2.fxaa          = false;

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
                settings.samplesMSAA = MSAASamples::_NONE;
            if (aaType == "msaa4")
                settings.samplesMSAA = MSAASamples::MSAA_x4;
            if (aaType == "msaa8")
                settings.samplesMSAA = MSAASamples::MSAA_x8;
            if (aaType == "fxaa")
            {
                settings2.fxaa       = true;
                settings.samplesMSAA = MSAASamples::_NONE;
            }

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

    init(settings, settings2);
    while (!m_window->get_window_should_close())
    {

        // I-O
        m_window->poll_events();

        tick();
    }
    m_renderer->shutdown(m_scene);
}

void VulkanRenderer::setup() {
    const std::string MESH_PATH(EXAMPLES_RESOURCES_PATH "meshes/");
    const std::string TEXTURE_PATH(EXAMPLES_RESOURCES_PATH "textures/");
    const std::string ENGINE_MESH_PATH(ENGINE_RESOURCES_PATH "meshes/");

    camera = new Camera();
    camera->set_position(Vec3(0.0f, 0.0f, -5.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    m_scene = new Scene(camera);

    m_scene->add(new PointLight());
    m_scene->get_lights()[0]->set_position({-3.0f, 3.0f, 0.0f});
    m_scene->get_lights()[0]->set_shadow_fov(115.0f);
    PointLight* light = new PointLight();
    light->set_area_of_effect(18);
    m_scene->add(light);
    m_scene->get_lights()[1]->set_position({3.0f, 6.5f, 10.0f});
    m_scene->get_lights()[1]->set_shadow_target({0.0f, 0.0f, 10.0f});
    m_scene->get_lights()[1]->set_shadow_fov(160.0f);
    m_scene->get_lights()[1]->set_color({0, 0.11, 0.55});
    m_scene->get_lights()[1]->set_intensity(2.5);
    m_scene->add(new PointLight());
    m_scene->get_lights()[2]->set_position({1.9f, 1.5f, 9.5f});
    m_scene->get_lights()[2]->set_shadow_target({1.1f, 0.0f, 9.5f});
    m_scene->get_lights()[2]->set_shadow_fov(110.0f);
    m_scene->get_lights()[2]->set_color({0.25, 0.13, 0});
    m_scene->get_lights()[2]->set_intensity(0.6);
    PointLight* light2 = new PointLight();
    light2->set_area_of_effect(0.440);
    m_scene->add(light2);
    m_scene->get_lights()[3]->set_position({2.5f, -1.6f, 9.4f});
    m_scene->get_lights()[3]->set_color({0.25, 0.13, 0});
    m_scene->get_lights()[3]->set_cast_shadows(false);

    Mesh*    toriiMesh = new Mesh();
    auto     toriiMat  = new PhysicallyBasedMaterial();
    Texture* toriiT    = new Texture();
    Tools::Loaders::load_texture(toriiT, TEXTURE_PATH + "torii_color.png");
    Texture* toriiN = new Texture();
    Tools::Loaders::load_texture(toriiN, TEXTURE_PATH + "torii_normal.png", TextureFormatType::NORMAL_FORMAT);
    Texture* toriiM = new Texture();
    Tools::Loaders::load_texture(toriiM, TEXTURE_PATH + "torii_mask.png");
    toriiMat->set_albedo_texture(toriiT);
    toriiMat->set_normal_texture(toriiN);
    toriiMat->set_metalness(0.05);
    toriiMat->set_roughness(0.5);
    // toriiMat->set_mask_texture(toriiM, UNREAL_ENGINE);
    toriiMesh->push_material(toriiMat);
    Tools::Loaders::load_3D_file(toriiMesh, MESH_PATH + "torii.obj");
    toriiMesh->set_name("Torii");
    toriiMesh->set_scale(0.2f);
    toriiMesh->set_position({1.6, -2.3, 6.1});
    toriiMesh->set_rotation({0.0, 28.0f, 8.0f});
    m_scene->add(toriiMesh);

    Mesh* terrainMesh = new Mesh();
    terrainMesh->set_scale(10.0);
    terrainMesh->set_position({0.0, -4.0, 0.0});
    Tools::Loaders::load_3D_file(terrainMesh, MESH_PATH + "terrain.obj");
    Texture* floorText = new Texture();
    Tools::Loaders::load_texture(floorText, TEXTURE_PATH + "floor_diffuse.jpg");

    Texture* floorNormalText = new Texture();
    Tools::Loaders::load_texture(floorNormalText, TEXTURE_PATH + "floor_normal.jpg", TextureFormatType::NORMAL_FORMAT);
    Texture* floorRoughText = new Texture();
    Tools::Loaders::load_texture(floorRoughText, TEXTURE_PATH + "floor_roughness.jpg");
    auto terrainMat = new PhysicallyBasedMaterial();
    terrainMat->set_albedo({0.43f, 0.28f, 0.23f});
    terrainMat->set_albedo_texture(floorText);
    terrainMat->set_normal_texture(floorNormalText);
    terrainMat->set_roughness_texture(floorRoughText);
    terrainMat->set_tile({25.0f, 25.0f});
    terrainMesh->push_material(terrainMat);
    terrainMesh->set_name("Terrain");
    m_scene->add(terrainMesh);

    auto lightMat = new UnlitMaterial();
    lightMat->set_color(glm::vec4(m_scene->get_lights()[0]->get_color(), 1.0f));
    m_lightDummy = new Mesh();
    Tools::Loaders::load_3D_file(m_lightDummy, ENGINE_MESH_PATH + "sphere.obj");
    m_lightDummy->push_material(lightMat);
    m_lightDummy->set_scale(0.5f);
    m_lightDummy->set_name("Light Gizmo");
    // m_scene->add(m_lightDummy);

    Mesh* kabutoMesh = new Mesh();
    Tools::Loaders::load_3D_file(kabutoMesh, MESH_PATH + "kabuto.obj");
    kabutoMesh->set_rotation(glm::vec3(0.0, 180, 0.0));
    auto     kabutoMat  = new PhysicallyBasedMaterial();
    Texture* kabutoText = new Texture();
    Tools::Loaders::load_texture(kabutoText, TEXTURE_PATH + "kabuto_color.png");
    kabutoMat->set_albedo_texture(kabutoText);
    kabutoMat->set_albedo({0.0, 1.0, 0.0});
    kabutoMat->set_metalness(0.8f);
    kabutoMat->set_roughness(0.4f);
    kabutoMesh->push_material(kabutoMat);
    kabutoMesh->set_name("Kabuto");
    m_scene->add(kabutoMesh);

    Mesh* templeMesh = new Mesh();

    Tools::Loaders::load_3D_file(templeMesh, MESH_PATH + "temple.obj");
    templeMesh->set_rotation(glm::vec3(0.0, 180, 0.0));
    auto     templeMat  = new PhysicallyBasedMaterial();
    Texture* templeText = new Texture();
    Tools::Loaders::load_texture(templeText, TEXTURE_PATH + "temple_diffuse.png");
    Texture* templeRText = new Texture();
    Tools::Loaders::load_texture(templeRText, TEXTURE_PATH + "temple_rough.png");
    Texture* templeMText = new Texture();
    Tools::Loaders::load_texture(templeMText, TEXTURE_PATH + "temple_metal.png");
    templeMat->set_albedo_texture(templeText);
    templeMat->set_metallic_texture(templeMText);
    templeMat->set_roughness_texture(templeRText);
    templeMat->set_albedo({0.0, 1.0, 0.0});
    templeMat->set_metalness(0.8f);
    templeMat->set_roughness(0.4f);
    templeMesh->push_material(templeMat);
    templeMesh->set_name("Temple");
    templeMesh->set_position({7.2, -2.87, 14.1});
    templeMesh->set_rotation({0.0, 230.0f, 0.0f});
    m_scene->add(templeMesh);

    Mesh* templeMesh2 = new Mesh();

    Tools::Loaders::load_3D_file(templeMesh2, MESH_PATH + "shrine.obj");
    templeMesh2->set_rotation(glm::vec3(0.0, 180, 0.0));
    auto     templeMat2  = new PhysicallyBasedMaterial();
    Texture* templeText2 = new Texture();
    Tools::Loaders::load_texture(templeText2, TEXTURE_PATH + "shrine_diffuse.png");
    Texture* templeRText2 = new Texture();
    Tools::Loaders::load_texture(templeRText2, TEXTURE_PATH + "shrine_rough.png");
    Texture* templeMText2 = new Texture();
    Tools::Loaders::load_texture(templeMText2, TEXTURE_PATH + "shrine_metal.png");
    templeMat2->set_albedo_texture(templeText2);
    templeMat2->set_metallic_texture(templeMText2);
    templeMat2->set_roughness_texture(templeRText2);
    templeMesh2->push_material(templeMat2);
    templeMesh2->set_name("Shrine");
    templeMesh2->set_position({0, -2.77, 14.1});
    templeMesh2->set_rotation({0.0, 160.0f, 0.0f});
    templeMesh2->set_scale(1.25);
    m_scene->add(templeMesh2);

    // Mesh* hair = new Mesh();
    // hair->load_file(MESH_PATH + "curly.hair", false);
    // hair->set_material(new HairMaterial());
    // hair->set_name("hair");
    // hair->set_scale(0.1f);
    // m_scene->add(hair);

    Mesh* lanternMesh = new Mesh();
    Tools::Loaders::load_3D_file(lanternMesh, MESH_PATH + "lantern.obj", false);
    auto     lanternMat = new PhysicallyBasedMaterial();
    Texture* lanternT   = new Texture();
    Tools::Loaders::load_texture(lanternT, TEXTURE_PATH + "lantern_diffuse.png");
    lanternMat->set_albedo_texture(lanternT);
    lanternMesh->push_material(lanternMat);
    lanternMesh->set_name("Lantern");
    lanternMesh->set_position({2.5, -1.67, 2.8});
    lanternMesh->set_rotation({0.0, 23.0f, 0.0f});
    lanternMesh->set_scale(1.25);
    lanternMat->set_roughness(0.8f);
    m_scene->add(lanternMesh);

    Mesh* lanternMesh2 = lanternMesh->clone();
    lanternMesh->set_position({-1.4, -2.47, 4.4});
    lanternMesh->set_rotation({0.0, 10.0f, 0.0f});
    m_scene->add(lanternMesh2);

    Mesh* lanternMesh3 = lanternMesh->clone();
    lanternMesh->set_position({1.4, -1.57, -0.8});
    lanternMesh->set_rotation({0.0, 9.0f, 0.0f});
    m_scene->add(lanternMesh3);

    Mesh* lanternMesh4 = lanternMesh->clone();
    lanternMesh->set_position({-2.4, -2.17, 1.0});
    lanternMesh->set_rotation({0.0, 14.0f, 0.0f});
    m_scene->add(lanternMesh4);

    Mesh* stoneMesh = new Mesh();
    Tools::Loaders::load_3D_file(stoneMesh, MESH_PATH + "stone_lantern.obj", false);
    auto     stoneMat      = new PhysicallyBasedMaterial();
    Texture* stonelanternT = new Texture();
    Tools::Loaders::load_texture(stonelanternT, TEXTURE_PATH + "stone_diffuse.png");
    stoneMat->set_albedo_texture(stonelanternT);
    stoneMesh->push_material(stoneMat);
    stoneMesh->set_name("Stone Lantern");
    stoneMesh->set_position({2.5, -2.67, 9.5});
    stoneMesh->set_rotation({0.0, 107.0f, 0.0f});
    stoneMesh->set_scale(1.5);
    stoneMat->set_roughness(0.9f);
    m_scene->add(stoneMesh);

    m_scene->set_ambient_color({0.2, 0.25, 0.61});

    TextureHDR* envMap = new TextureHDR();
    Tools::Loaders::load_texture(envMap, TEXTURE_PATH + "cloudy.hdr");
    Skybox* sky = new Skybox(envMap);
    sky->set_color_intensity(0.25f);
    m_scene->set_skybox(sky);

    m_controller = new Tools::Controller(camera, m_window);
}

void VulkanRenderer::setup_gui() {
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

    // m_renderer->set_gui_overlay(m_interface.overlay);
}

void VulkanRenderer::update() {
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
    m_lightDummy->set_position(light->get_position());
    dynamic_cast<UnlitMaterial*>(m_lightDummy->get_material())->set_color(glm::vec4(light->get_color(), 1.0f));

    m_interface.object->set_object(m_interface.scene->get_selected_object());
}

void VulkanRenderer::tick() {
    float currentTime      = (float)m_window->get_time_elapsed();
    m_time.delta           = currentTime - m_time.last;
    m_time.last            = currentTime;
    m_time.framesPerSecond = 1.0f / m_time.delta;

    update();

    m_interface.overlay->render();
    m_renderer->render(m_scene);
}
