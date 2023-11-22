#pragma once
#include "gfx/vk_renderer.h"
#include "utilities/vk_controller.h"
// IMPORT THAT AS LIBRARY IN THE FUTURE

/**
 * Example app
 */
class VulkanEngine
{
    vkeng::Window *m_window;
    vkeng::Renderer *m_renderer;
    std::vector<vkeng::Mesh *> meshes;
    vkeng::Camera *camera;
    vkeng::CameraController *m_controller;
    // vkeng::Scene m_scene;
    // imgui::gui m_gui;

public:
    void
    init();

    void run();

private:
    void setup();

    void tick();

#pragma region Input Management

    void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        {
            m_renderer->set_shader();
        }

        if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
        {
            m_window->set_fullscreen(m_window->is_fullscreen() ? false : true);
        }

        m_controller->handle_keyboard(window, 1.0);
    }

    void mouse_callback(GLFWwindow* w, double xpos, double ypos)
    {
        m_controller->handle_mouse(xpos,ypos);
    }

    void window_resize_callback(GLFWwindow *window, int width, int height)
    {
        m_window->set_size(width, height);
    }

#pragma endregion
};