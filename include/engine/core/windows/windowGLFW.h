/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef WINDOW_GLFW_H
#define WINDOW_GLFW_H

#include <engine/core/windows/window.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{
/**
 * GLFW as windowing manage system.
 */
class WindowGLFW : public Window
{
  private:
    GLFWwindow *m_handle{nullptr};

  public:
    WindowGLFW(const std::string t, uint32_t w, uint32_t h, bool resizable = true, bool fullscreen = false)
        : Window(t, w, h, resizable, fullscreen)
    {
    }

    void init();

    inline void destroy()
    {
        glfwDestroyWindow(m_handle);
    }
    inline void get_handle(void * &handlePtr) const
    {
        handlePtr = m_handle;
    }
    inline WindowingSystem get_windowing_system() const
    {
        return WindowingSystem::GLFW;
    }
    inline int get_window_should_close() const
    {
        return glfwWindowShouldClose(m_handle);
    }

    inline void set_window_should_close(bool op)
    {
        glfwSetWindowShouldClose(m_handle, op);
    }

    void set_fullscreen(bool t);

    void update_framebuffer();

    inline void set_title(const char *title)
    {
        Window::set_title(title);
        glfwSetWindowTitle(m_handle, title);
    }
    inline void set_position(math::ivec2 p)
    {
        Window::set_position(p);
        glfwSetWindowPos(m_handle, p.x, p.y);
    }

    inline void poll_events()
    {
        glfwPollEvents();
    }

    inline double get_time_elapsed()
    {
        return glfwGetTime();
    }
};

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
#endif