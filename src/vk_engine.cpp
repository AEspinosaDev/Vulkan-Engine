#include "vk_engine.h"

void VulkanEngine::init()
{
    m_window = new vkeng::Window("VK Engine", 800, 600);
    m_renderer = new vkeng::Renderer(m_window);
    m_renderer->init();

    setup();
}

void VulkanEngine::run()
{
    init();
    while (!glfwWindowShouldClose(m_window->get_window_obj()))
    {
        // I-O
        m_window->poll_events();
        tick();
        m_renderer->render(meshes, camera);
    }
    m_renderer->shutdown();
}

void VulkanEngine::setup()
{

    vkeng::Geometry *g = new vkeng::Geometry();
    g->fill({{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
             {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
             {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
             {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}},
            {0, 1, 2, 2, 3, 0});
    vkeng::Mesh *m = new vkeng::Mesh(g);

    meshes.push_back(m);

    camera = new vkeng::Camera();

    camera->set_position(glm::vec3(0.0f, 0.0f, -1.0f));
    camera->set_far(200.0f);
    camera->set_near(0.01f);
    camera->set_field_of_view(70.0f);

    m_controller = new vkeng::CameraController(camera);

    // WINDOW CALLBACKS
    glfwSetWindowUserPointer(m_window->get_window_obj(), this);

    glfwSetFramebufferSizeCallback(m_window->get_window_obj(), [](GLFWwindow *w, int width, int heigth)
                                   { static_cast<VulkanEngine *>(glfwGetWindowUserPointer(w))->window_resize_callback(w, width, heigth); });
    glfwSetKeyCallback(m_window->get_window_obj(), [](GLFWwindow *w, int key, int scancode, int action, int mods)
                       { static_cast<VulkanEngine *>(glfwGetWindowUserPointer(w))->keyboard_callback(w, key, scancode, action, mods); });
    //     glfwSetCursorPosCallback(m_window->get_window_obj(), [](GLFWwindow* w, double xpos, double ypos)
    // 	{
    //         //  { static_cast<VulkanEngine *>(glfwGetWindowUserPointer(w)) });
    // 	};
}

void VulkanEngine::tick()
{
}
