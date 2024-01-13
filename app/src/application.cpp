#include "application.h"
#include <filesystem>

void VulkanRenderer::init()
{
    m_window = new vke::Window("VK Engine", 800, 600);

    m_window->init();

    m_window->set_window_size_callback(std::bind(&VulkanRenderer::window_resize_callback, this, std::placeholders::_1,
                                                 std::placeholders::_2));
    m_window->set_mouse_callback(std::bind(&VulkanRenderer::mouse_callback, this, std::placeholders::_1,
                                           std::placeholders::_2));
    m_window->set_key_callback(std::bind(&VulkanRenderer::keyboard_callback, this, std::placeholders::_1,
                                         std::placeholders::_2, std::placeholders::_3,
                                         std::placeholders::_4));

    vke::RendererSettings settings{};
    settings.AAtype = vke::AntialiasingType::_MSAA_8;
    settings.clearColor = glm::vec4(0.02, 0.02, 0.02, 1.0);

    m_renderer = new vke::Renderer(m_window, settings);

    setup();
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

    m_scene->set_light(new vke::PointLight());
    m_scene->get_light()->set_position({-3.0f, 3.0f, 0.0f});

    vke::Geometry *quadGeom = new vke::Geometry();
    quadGeom->fill({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}},
                   {0, 1, 2, 2, 3, 0});

    auto mat = new vke::PhysicalBasedMaterial();
    mat->set_albedo({0.43f, 0.28f, 0.23f, 1.0});
    mat->set_metalness(0.0f);
    mat->set_roughness(0.9f);

    auto mat2 = new vke::PhysicalBasedMaterial();
    mat2->set_albedo({0.0, 1.0, 0.0, 1.0});
    mat2->set_metalness(0.8f);
    mat2->set_roughness(0.3f);

    auto mat3 = new vke::PhysicalBasedMaterial();
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
    mat->set_tile({15.0f, 15.0f});

    vke::Texture *woodText = new vke::Texture();
    woodText->load_image(textDir + "wood_diffuse.jpg");
    vke::Texture *woodNormalText = new vke::Texture();
    woodNormalText->load_image(textDir + "wood_normal.jpg");
    mat3->set_albedo_texture(woodText);
    mat3->set_normal_texture(woodNormalText);

    m_controller = new vke::Controller(camera);
}

void VulkanRenderer::update()
{
    m_controller->handle_keyboard(m_window->get_window_obj(), 0, 0, m_deltaTime);

    // Rotate the vector around the ZX plane
    float rotationAngle = glm::radians(10.0f * m_deltaTime);
    auto light = m_scene->get_light();
    float _x = light->get_position().x * cos(rotationAngle) - light->get_position().z * sin(rotationAngle);
    float _z = light->get_position().x * sin(rotationAngle) + light->get_position().z * cos(rotationAngle);

    light->set_position({_x, light->get_position().y, _z});
    m_lightDummy->set_position(light->get_position());
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
