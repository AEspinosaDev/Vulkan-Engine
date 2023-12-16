#ifndef VK_APPLICATION_H
#define VK_APPLICATION_H

#include <engine/vk_renderer.h>
#include <engine/utilities/vk_controller.h>
#include <engine/materials/vk_basic_unlit.h>
#include <engine/materials/vk_basic_phong.h>
#include <chrono>
#include "config.h"


/**
 * Example app
 */
class VulkanRenderer
{
    vke::Window *m_window;
    vke::Renderer *m_renderer;
    vke::Scene* m_scene;
    vke::Camera *camera;
    vke::Controller *m_controller;
    
    float m_deltaTime{0.0f};
    float m_lastTime{0.0f};
    // std::chrono::steady_clock::time_point m_lastTime;

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
         if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            m_window->set_window_should_close(true);
        }

        if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
        {
            m_window->set_fullscreen(m_window->is_fullscreen() ? false : true);
        }

        m_controller->handle_keyboard(window,key,action, m_deltaTime);
    }

    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        m_controller->handle_mouse(window,(float)xpos,(float)ypos);
    }

    void window_resize_callback(GLFWwindow *window, int width, int height)
    {
        m_window->set_size(width, height);
    }

#pragma endregion
};

#endif