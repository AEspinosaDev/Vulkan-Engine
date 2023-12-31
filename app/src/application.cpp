#include "application.h"
#include <filesystem>

void VulkanRenderer::init()
{
    m_window = new vke::Window("VK Engine", 800, 600);
    m_renderer = new vke::Renderer(m_window);
    m_renderer->init();

    m_window->set_window_size_callback(std::bind(&VulkanRenderer::window_resize_callback, this, std::placeholders::_1,
                                                 std::placeholders::_2));
    m_window->set_mouse_callback(std::bind(&VulkanRenderer::mouse_callback, this, std::placeholders::_1,
                                           std::placeholders::_2));
    m_window->set_key_callback(std::bind(&VulkanRenderer::keyboard_callback, this, std::placeholders::_1,
                                         std::placeholders::_2, std::placeholders::_3,
                                         std::placeholders::_4));

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
    camera = new vke::Camera();
    m_scene = new vke::Scene(camera);

    camera->set_position(glm::vec3(0.0f, 0.0f, -5.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    // m_scene->set_rotation({0.7,1.5, 0.0});

    vke::Geometry *quadGeom = new vke::Geometry();
    quadGeom->fill({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                    {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}},
                   {0, 1, 2, 2, 3, 0});

    auto mat = new vke::BasicPhongMaterial();
    mat->set_color({1.0, 0.0, 0.0, 1.0});
    auto mat2 = new vke::BasicPhongMaterial();
    mat2->set_color({0.0, 1.0, 0.0, 1.0});
    auto mat3 = new vke::BasicPhongMaterial();
    mat3->set_color({0.0, 0.0, 1.0, 1.0});

    vke::Mesh *m = new vke::Mesh(quadGeom, mat);
    vke::Mesh *m2 = new vke::Mesh();
    m2->set_material(mat2);

    std::string meshDir(MODEL_DIR);
    std::string engineMeshDir(VK_MODEL_DIR);
    m2->load_file(meshDir + "kabuto.obj");
    m2->set_rotation(glm::vec3(0.0, 3.14, 0.0));

    vke::Mesh *m3 = new vke::Mesh();
    m3->set_material(mat3);

    m->set_scale(10.0);
    m->set_position({0.0, -4.0, 0.0});
    m->load_file(meshDir + "terrain.obj", true);
    m3->set_position({-2.0, 2.0, 2.0});
    m3->load_file(engineMeshDir + "cube.obj");

    m_scene->add(m);
    m_scene->add(m2);
    m_scene->add(m3);

    std::string textDir(TEXTURE_DIR);

    vke::Texture *text = new vke::Texture();
    text->load_image(textDir + "wood_diffuse.jpg");

    vke::Texture *text2 = new vke::Texture();
    text2->load_image(textDir + "land.png");

    mat3->set_color_texture(text);
    mat->set_color_texture(text2);

    m_controller = new vke::Controller(camera);
}

void VulkanRenderer::tick()
{
    float currentTime = (float)vke::Window::get_time_elapsed();
    m_deltaTime = currentTime - m_lastTime;
    m_lastTime = currentTime;
    float fps = 1.0f / m_deltaTime;

    m_controller->handle_keyboard(m_window->get_window_obj(), 0, 0, m_deltaTime);

    m_renderer->render(m_scene);
}
