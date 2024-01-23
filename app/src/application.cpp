#include "application.h"
#include <filesystem>

void VulkanRenderer::init()
{
    m_window = new vke::Window("VK Engine", 1200, 900);

    m_window->init();

    m_window->set_window_size_callback(std::bind(&VulkanRenderer::window_resize_callback, this, std::placeholders::_1,
                                                 std::placeholders::_2));
    m_window->set_mouse_callback(std::bind(&VulkanRenderer::mouse_callback, this, std::placeholders::_1,
                                           std::placeholders::_2));
    m_window->set_key_callback(std::bind(&VulkanRenderer::keyboard_callback, this, std::placeholders::_1,
                                         std::placeholders::_2, std::placeholders::_3,
                                         std::placeholders::_4));

    vke::RendererSettings settings{};
    settings.AAtype = vke::AntialiasingType::MSAA_x8;
    settings.clearColor = glm::vec4(0.02, 0.02, 0.02, 1.0);
    settings.enableUI = true;

    m_renderer = new vke::Renderer(m_window, settings);

    setup();

    setup_gui();
}

void VulkanRenderer::run()
{
    init();
    while (!m_window->get_window_should_close())
    {

        // I-O
        vke::Window::poll_events();

        tick();
    }
    m_renderer->shutdown();
}

void VulkanRenderer::setup()
{
    std::string meshDir(MODEL_DIR);
    std::string engineMeshDir(VK_MODEL_DIR);

    camera = new vke::Camera();
    camera->set_position(glm::vec3(0.0f, 0.0f, -5.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    m_scene = new vke::Scene(camera);

    // m_scene->set_light(new vke::PointLight());
    m_scene->add(new vke::PointLight());
    m_scene->get_light()->set_position({-3.0f, 3.0f, 0.0f});

    vke::Geometry *quadGeom = new vke::Geometry();
    quadGeom->fill({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}},
                   {0, 1, 2, 2, 3, 0});

    auto mat = new vke::PhysicallyBasedMaterial();
    mat->set_albedo({0.43f, 0.28f, 0.23f, 1.0});
    mat->set_metalness(0.0f);
    mat->set_roughness(0.9f);

    auto mat2 = new vke::PhysicallyBasedMaterial();
    mat2->set_albedo({0.0, 1.0, 0.0, 1.0});
    mat2->set_metalness(0.8f);
    mat2->set_roughness(0.3f);

    auto mat3 = new vke::PhysicallyBasedMaterial();
    mat3->set_albedo({0.0, 0.0, 0.0, 1.0});

    auto lightMat = new vke::UnlitMaterial();
    lightMat->set_color(glm::vec4(m_scene->get_light()->get_color(), 1.0f));
    m_lightDummy = new vke::Mesh();
    m_lightDummy->load_file(engineMeshDir + "sphere.obj");
    m_lightDummy->set_material(lightMat);
    m_lightDummy->set_scale(0.5f);

    vke::Mesh *m = new vke::Mesh(quadGeom, mat);
    m->set_scale(10.0);
    m->set_position({0.0, -4.0, 0.0});
    m->load_file(meshDir + "terrain.obj", true);

    vke::Mesh *m2 = new vke::Mesh();
    m2->set_material(mat2);
    m2->load_file(meshDir + "kabuto.obj");
    m2->set_rotation(glm::vec3(0.0, 3.14, 0.0));

    vke::Mesh *m3 = new vke::Mesh();
    m3->set_material(mat3);
    m3->set_position({-3.0, 2.0, 3.0});
    m3->load_file(engineMeshDir + "cube.obj");

    m_scene->add(m);
    m_scene->add(m2);
    m_scene->add(m3);
    m_scene->add(m_lightDummy);

    std::string textDir(TEXTURE_DIR);

    vke::Texture *floorText = new vke::Texture();
    floorText->load_image(textDir + "floor_diffuse.jpg");
    vke::Texture *floorNormalText = new vke::Texture();
    floorNormalText->load_image(textDir + "floor_normal.jpg");
    vke::Texture *floorRoughText = new vke::Texture();
    floorRoughText->load_image(textDir + "floor_roughness.jpg");
    mat->set_albedo_texture(floorText);
    mat->set_normal_texture(floorNormalText);
    mat->set_roughness_texture(floorRoughText);
    mat->set_tile({25.0f, 25.0f});

    vke::Texture *woodText = new vke::Texture();
    woodText->load_image(textDir + "wood_diffuse.jpg");
    vke::Texture *woodNormalText = new vke::Texture();
    woodNormalText->load_image(textDir + "wood_normal.jpg");
    mat3->set_albedo_texture(woodText);
    mat3->set_normal_texture(woodNormalText);

    m_controller = new vke::Controller(camera);
}

void VulkanRenderer::setup_gui()
{
    m_interface.overlay = new vke::GUIOverlay((float)m_window->get_extent()->width, (float)m_window->get_extent()->height,vke::GuiColorProfileType::DARK);

    vke::Panel *tutorialPanel = new vke::Panel("TUTORIAL", 0,0.8f, 0.2f, 0.2f, vke::PanelWidgetFlags::NoMove, false, true);

    tutorialPanel->add_child(new vke::Space());
    tutorialPanel->add_child(new vke::Separator("CONTROLS"));
    tutorialPanel->add_child(new vke::Separator());
    tutorialPanel->add_child(new vke::TextLine("WASD: move camera.", vke::TextWidgetType::BULLET));
    tutorialPanel->add_child(new vke::TextLine("QE: move up/down camera.", vke::TextWidgetType::BULLET));
    tutorialPanel->add_child(new vke::TextLine("Mouse + Left: rotate camera.", vke::TextWidgetType::BULLET));
    tutorialPanel->add_child(new vke::TextLine("F11: toggle fullscreen/windowed mode.", vke::TextWidgetType::BULLET));
    tutorialPanel->add_child(new vke::TextLine("Esc: exit application.", vke::TextWidgetType::BULLET));
    tutorialPanel->add_child(new vke::Space());
    tutorialPanel->add_child(new vke::Separator());
    tutorialPanel->add_child(new vke::TextLine("Enjoy changing the parameters!"));

    m_interface.overlay->add_panel(tutorialPanel);
    m_interface.tutorial = tutorialPanel;

    vke::Panel *explorerPanel = new vke::Panel("EXPLORER", 0, 0, 0.2f, 0.7f, vke::PanelWidgetFlags::NoMove, false);
    m_interface.scene = new vke::SceneExplorerWidget(m_scene);
    explorerPanel->add_child(m_interface.scene);
    explorerPanel->add_child(new vke::Space());
    explorerPanel->add_child(new vke::Separator("GLOBAL SETTINGS"));
    explorerPanel->add_child(new vke::Separator());
    explorerPanel->add_child(new vke::TextLine(" Application average (No cap)"));
    explorerPanel->add_child(new vke::Space());
    explorerPanel->add_child(new vke::Profiler());
    explorerPanel->add_child(new vke::Space());
    explorerPanel->add_child(new vke::Separator());

    m_interface.overlay->add_panel(explorerPanel);
    m_interface.explorer = explorerPanel;

    vke::Panel *propertiesPanel = new vke::Panel("PROPERTIES", 0.75f, 0, 0.25f, 0.8f, vke::PanelWidgetFlags::NoMove, true);
    m_interface.object = new vke::ObjectExplorerWidget();
    propertiesPanel->add_child(m_interface.object);

    m_interface.overlay->add_panel(propertiesPanel);
    m_interface.properties = propertiesPanel;

    m_renderer->set_gui_overlay(m_interface.overlay);
}

void VulkanRenderer::update()
{
    if (!m_interface.overlay->wants_to_handle_input())
        m_controller->handle_keyboard(m_window->get_window_obj(), 0, 0, m_deltaTime);

    // Rotate the vector around the ZX plane
    float rotationAngle = glm::radians(10.0f * m_deltaTime);
    auto light = m_scene->get_light();
    float _x = light->get_position().x * cos(rotationAngle) - light->get_position().z * sin(rotationAngle);
    float _z = light->get_position().x * sin(rotationAngle) + light->get_position().z * cos(rotationAngle);

    light->set_position({_x, light->get_position().y, _z});
    m_lightDummy->set_position(light->get_position());
    dynamic_cast<vke::UnlitMaterial*>(m_lightDummy->get_material())->set_color(glm::vec4(light->get_color(),1.0f));

    m_interface.object->set_object(m_interface.scene->get_selected_object());
}

void VulkanRenderer::tick()
{
    float currentTime = (float)vke::Window::get_time_elapsed();
    m_deltaTime = currentTime - m_lastTime;
    m_lastTime = currentTime;
    float fps = 1.0f / m_deltaTime;

    update();

    m_renderer->render(m_scene);
}
