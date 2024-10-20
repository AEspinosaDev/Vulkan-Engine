/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef WINDOW_H
#define WINDOW_H

#include <engine/common.h>
#include <engine/graphics/swapchain.h>
#include <engine/graphics/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

/**
 * Class containing all functionality related to a window. It uses GLFW as context creator.
 */
class Window
{
  private:
    std::string m_title{};

    GLFWwindow *m_handle{nullptr};

    Extent2D m_extent;

    bool m_initialized{false};
    bool m_resized{false};
    bool m_resizeable;
    bool m_fullscreen;

    // Windowed
    Extent2D m_windowedExtent;
    math::ivec2 m_screenPos = math::ivec2(45, 45);

    // Callbacks
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void(int, int)> m_windowSizeCallback;
    std::function<void(double, double)> m_mouseCallBack;

  public:
    Window(const std::string t, uint32_t w, uint32_t h, bool resizable = true, bool fullscreen = false)
        : m_title(t), m_extent(VkExtent2D{w, h}), m_windowedExtent({w, h}), m_resizeable{resizable},
          m_fullscreen{fullscreen}
    {
    }

    void init();

    inline void destroy()
    {
        glfwDestroyWindow(m_handle);
    }

    inline void set_size(int w, int h)
    {
        if (!m_fullscreen)
        {
            m_windowedExtent.width = w;
            m_windowedExtent.height = h;
        }
        m_resized = true;
    }
    inline Extent2D get_extent() const
    {
        if (!m_fullscreen)
        {
            return m_windowedExtent;
        }
        else
            return m_extent;
    }

    inline bool initialized()
    {
        return m_initialized;
    }

    inline bool is_resized()
    {
        return m_resized;
    }

    inline void set_resized(bool r)
    {
        m_resized = r;
    }

    inline bool is_fullscreen()
    {
        return m_fullscreen;
    }

    inline void set_fullscreen(bool t)
    {
        if (!m_initialized)
            return;
        m_fullscreen = t;
        if (!m_fullscreen)
        {
            glfwSetWindowMonitor(m_handle, NULL, m_screenPos.x, m_screenPos.y, m_windowedExtent.width,
                                 m_windowedExtent.height, GLFW_DONT_CARE);
        }
        else
        {
            const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(m_handle, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            m_extent.width = mode->width;
            m_extent.height = mode->height;
        }
    }
    inline bool is_resizable()
    {
        return m_resizeable;
    }

    inline void set_resizeable(bool t)
    {
        m_resizeable = t; /*glfwsetwindowresize(GLFW_RESIZABLE, t);*/
    }

    inline math::ivec2 get_position()
    {
        return m_screenPos;
    }

    inline void set_position(math::ivec2 p)
    {
        if (!m_initialized)
            return;
        m_screenPos = p;
        glfwSetWindowPos(m_handle, p.x, p.y);
    }
    inline GLFWwindow *const get_handle() const
    {
        return m_handle;
    }

    inline int get_window_should_close() const
    {
        return glfwWindowShouldClose(m_handle);
    }

    inline void set_window_should_close(bool op)
    {
        glfwSetWindowShouldClose(m_handle, op);
    }

    inline void set_title(const char *title)
    {
        if (!m_initialized)
            return;
        m_title = title;
        glfwSetWindowTitle(m_handle, title);
    }
    inline std::string get_title()
    {
        return m_title;
    }

    inline static void poll_events()
    {
        glfwPollEvents();
    }

    inline static double get_time_elapsed()
    {
        return glfwGetTime();
    }

    inline void set_key_callback(std::function<void(int, int, int, int)> callback)
    {
        m_keyCallback = callback;
    }

    inline void set_window_size_callback(std::function<void(int, int)> callback)
    {
        m_windowSizeCallback = callback;
    }

    inline void set_mouse_callback(std::function<void(double, double)> callback)
    {
        m_mouseCallBack = callback;
    }
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
#endif