#include <engine/core/windows/windowGLFW.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {
void WindowGLFW::init() {
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

    glfwSetKeyCallback(m_handle, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        WindowGLFW* instance = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(w));
        if (instance->m_keyCallback)
        {
            instance->m_keyCallback(key, scancode, action, mods);
        }
    });

    glfwSetFramebufferSizeCallback(m_handle, [](GLFWwindow* w, int width, int height) {
        WindowGLFW* instance = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(w));
        if (instance->m_windowSizeCallback)
        {
            instance->m_windowSizeCallback(width, height);
        }
    });

    glfwSetCursorPosCallback(m_handle, [](GLFWwindow* w, double x, double y) {
        WindowGLFW* instance = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(w));
        if (instance->m_mouseCallBack)
        {
            instance->m_mouseCallBack(x, y);
        }
    });

    m_initialized = true;
}

void WindowGLFW::set_fullscreen(bool t) {
    if (!m_initialized)
        return;
    m_fullscreen = t;
    if (!m_fullscreen)
    {
        glfwSetWindowMonitor(m_handle,
                             NULL,
                             m_screenPos.x,
                             m_screenPos.y,
                             m_windowedExtent.width,
                             m_windowedExtent.height,
                             GLFW_DONT_CARE);
    } else
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(m_handle, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
        m_extent.width  = mode->width;
        m_extent.height = mode->height;
    }
}

void WindowGLFW::update_framebuffer() {
    // GLFW update framebuffer
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_handle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_handle, &width, &height);
        glfwWaitEvents();
    }
}
void WindowGLFW::set_window_icon(const char* iconPath) {
    int width, height, channels;
    // Load the image using stb_image
    unsigned char* pixels = stbi_load(iconPath, &width, &height, &channels, 4); // Force 4 channels (RGBA)
    if (!pixels)
    {
        ERR_LOG("failed to load window ICON");
        return;
    }

    GLFWimage icon;
    icon.width  = width;
    icon.height = height;
    icon.pixels = pixels;

    glfwSetWindowIcon(m_handle, 1, &icon);

    stbi_image_free(pixels);
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END