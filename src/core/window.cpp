#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Window::init()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Set for vulkan context

    glfwWindowHint(GLFW_RESIZABLE, m_resizeable);

    m_GLFWwindow = glfwCreateWindow(m_extent.width, m_extent.height, m_title.c_str(), nullptr, nullptr);

    if (!m_GLFWwindow)
    {
        glfwTerminate();
        ERR_LOG("Failed to create GLFW window");
    }

    glfwSetWindowPos(m_GLFWwindow, (int)m_screenPos.x, (int)m_screenPos.y);

    glfwSetWindowUserPointer(m_GLFWwindow, this);

    glfwSetKeyCallback(m_GLFWwindow, [](GLFWwindow *w, int key, int scancode, int action, int mods)
                       {
            Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (instance->m_keyCallback) {
                instance->m_keyCallback(key, scancode, action, mods);
            } });

    glfwSetFramebufferSizeCallback(m_GLFWwindow, [](GLFWwindow *w, int width, int height)
                                   {
            Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (instance->m_windowSizeCallback) {
                instance->m_windowSizeCallback(width, height);
            } });

    glfwSetCursorPosCallback(m_GLFWwindow, [](GLFWwindow *w, double x, double y)
                             {
            Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(w));
            if (instance->m_mouseCallBack) {
                instance->m_mouseCallBack(x,y);
            } });

    m_initialized = true;
}

void create_surface(VkInstance &instance, Window *window)
{
    VK_CHECK(glfwCreateWindowSurface(instance, window->m_GLFWwindow, nullptr, &window->m_surface));
}


VULKAN_ENGINE_NAMESPACE_END