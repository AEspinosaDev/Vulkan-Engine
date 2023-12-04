#include "engine/vk_window.h"

namespace vke
{

    void Window::init()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Set for vulkan context

        glfwWindowHint(GLFW_RESIZABLE, m_resizeable);

        m_GLFWwindow = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

        glfwSetWindowPos(m_GLFWwindow, (int)m_screenPos.x, (int)m_screenPos.y);
    }

    void Window::set_keyboard_callback(std::function<void(GLFWwindow *, int, int, int, int)> callback)
    {
        // glfwSetKeyCallback(m_GLFWwindow, [](GLFWwindow *w, int key, int scancode, int action, int mods)
        //                    { callback(w, key, scancode, action, mods); });
    }
}