#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Window::init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Set for vulkan context

    glfwWindowHint(GLFW_RESIZABLE, m_resizeable);

    m_handle = glfwCreateWindow(m_extent.width, m_extent.height, m_title.c_str(), nullptr, nullptr);

    if (!m_handle)
    {
        glfwTerminate();
        ERR_LOG("Failed to create GLFW window");
    }

    glfwSetWindowPos(m_handle, (int)m_screenPos.x, (int)m_screenPos.y);

    glfwSetWindowUserPointer(m_handle, this);

    glfwSetKeyCallback(m_handle, [](GLFWwindow *w, int key, int scancode, int action, int mods)
                       {
            Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (instance->m_keyCallback) {
                instance->m_keyCallback(key, scancode, action, mods);
            } });

    glfwSetFramebufferSizeCallback(m_handle, [](GLFWwindow *w, int width, int height)
                                   {
            Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (instance->m_windowSizeCallback) {
                instance->m_windowSizeCallback(width, height);
            } });

    glfwSetCursorPosCallback(m_handle, [](GLFWwindow *w, double x, double y)
                             {
            Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (instance->m_mouseCallBack) {
                instance->m_mouseCallBack(x,y);
            } });

    m_initialized = true;
}


VULKAN_ENGINE_NAMESPACE_END