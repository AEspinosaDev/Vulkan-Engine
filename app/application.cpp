#include "application.h"

void VulkanRenderer::init()
{
    m_window = new vke::Window("VK Engine", 800, 600);
    m_renderer = new vke::Renderer(m_window);
    m_renderer->init();

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

    camera->set_position(glm::vec3(0.0f, 0.0f, -1.0f));
    camera->set_far(100.0f);
    camera->set_near(0.1f);
    camera->set_field_of_view(70.0f);

    // m_scene->set_position({0.0, 10.0, 0.0});
    // m_scene->set_rotation({0.7,1.5, 0.0});

    vke::Geometry *g = new vke::Geometry();
    g->fill({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
             {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
             {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
             {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}},
            {0, 1, 2, 2, 3, 0});
    auto mat = new vke::BasicUnlitMaterial();
    mat->set_color({1.0, 0.0, 0.0, 1.0});
    vke::Mesh *m = new vke::Mesh(g, mat);

    // meshes.push_back(m);
    m_scene->add(m);
    m->set_scale(5.0);
    m->set_position({0.0, -1.0, 0.0});
    m->set_rotation(glm::radians(glm::vec3{90.0, 0.0, 0.0}));

    vke::Geometry *g2 = new vke::Geometry();
    g2->fill({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
              {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
              {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
              {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}},
             {0, 1, 2, 2, 3, 0});
    auto mat2 = new vke::BasicUnlitMaterial();
    mat2->set_color({0.0, 1.0, 0.0, 1.0});
    vke::Mesh *m2 = new vke::Mesh(g2, mat2);

    m_scene->add(m2);

    vke::Geometry *g3 = new vke::Geometry();
    g3->fill({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
              {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
              {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
              {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}},
             {0, 1, 2, 2, 3, 0});
    auto mat3 = new vke::BasicUnlitMaterial();
    mat3->set_color({0.0, 0.0, 1.0, 1.0});
    vke::Mesh *m3 = new vke::Mesh(g3, mat3);

    m_scene->add(m3);

    m3->set_position({-2.0, 2.0, 2.0});
    m3->load_file("../core/resources/meshes/cube.obj");

    m_controller = new vke::Controller(camera);
    // m_controller = new vke::Controller(camera, vke::ORBITAL);

    // m_renderer.
    // m_window->set_keyboard_callback(&VulkanRenderer::keyboard_callback);

    // WINDOW CALLBACKS
    glfwSetWindowUserPointer(m_window->get_window_obj(), this);

    glfwSetFramebufferSizeCallback(m_window->get_window_obj(), [](GLFWwindow *w, int width, int heigth)
                                   { static_cast<VulkanRenderer *>(glfwGetWindowUserPointer(w))->window_resize_callback(w, width, heigth); });
    // glfwSetKeyCallback(m_window->get_window_obj(), [](GLFWwindow *w, int key, int scancode, int action, int mods)
    //                    { static_cast<VulkanRenderer *>(glfwGetWindowUserPointer(w))->keyboard_callback(w, key, scancode, action, mods); });
    glfwSetCursorPosCallback(m_window->get_window_obj(), [](GLFWwindow *w, double xpos, double ypos)
                             { static_cast<VulkanRenderer *>(glfwGetWindowUserPointer(w))->mouse_callback(w, xpos, ypos); });

    // m_lastTime = std::chrono::high_resolution_clock::now();
}

void VulkanRenderer::tick()
{
    // auto currentTime = std::chrono::high_resolution_clock::now();
    float currentTime = (float)vke::Window::get_time_elapsed();
    // m_deltaTime = std::chrono::duration<float, std::chrono::period>(currentTime - m_lastTime).count();
    m_deltaTime = currentTime - m_lastTime;
    m_lastTime = currentTime;
    float fps = 1.0 / m_deltaTime;

    keyboard_callback(m_window->get_window_obj(), 0, 0, 0, 0);

    std::string newTitle = "VK ENGINE         fps = " + std::to_string(fps);
    m_window->set_title(newTitle.c_str());

    m_renderer->render(m_scene);

    // std::cout << m_scene->get_rotation().x << " " << m_scene->get_rotation().y << std::endl;
}
