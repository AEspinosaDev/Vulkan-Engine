#include "vk_window.h"

namespace vke {

    void Window::init()
    {
        glfwInit();
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Set for vulkan context

        glfwWindowHint(GLFW_RESIZABLE, m_resizeable);

        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

        glfwSetWindowPos(m_window, (int)m_screenPos.x, (int)m_screenPos.y);
    }

}