/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef WINDOW_H
#define WINDOW_H

#include <functional>
#include <engine/common.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

/**
 * Class containing all functionality related to a window.
 */
class IWindow
{
  protected:
    std::string m_title{};
    Extent2D m_extent;

    bool m_initialized{false};
    bool m_resized{false};
    bool m_resizeable;
    bool m_fullscreen;

    // Windowed Mode Data
    Extent2D m_windowedExtent;
    math::ivec2 m_screenPos = math::ivec2(45, 45);

    // Callbacks
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void(int, int)> m_windowSizeCallback;
    std::function<void(double, double)> m_mouseCallBack;

  public:
    IWindow(const std::string t, uint32_t w, uint32_t h, bool resizable = true, bool fullscreen = false)
        : m_title(t), m_extent(VkExtent2D{w, h}), m_windowedExtent({w, h}), m_resizeable{resizable},
          m_fullscreen{fullscreen}
    {
    }

    virtual void init() = 0;

    virtual void destroy() = 0;

    virtual void get_handle(void * &handlePtr) const = 0;

    virtual void set_fullscreen(bool t) = 0;

    virtual int get_window_should_close() const = 0;

    virtual void set_window_should_close(bool op) = 0;

    virtual void poll_events() = 0;

    virtual double get_time_elapsed() = 0;

    virtual void update_framebuffer() = 0;

    virtual WindowingSystem get_windowing_system() const = 0;

    virtual void set_window_icon(const char* iconPath) = 0;

    virtual bool get_key_state(int keyCode, int state) const{return 0;}

    virtual inline void set_position(math::ivec2 p)
    {
        if (!m_initialized)
            return;
        m_screenPos = p;
    }

    virtual void set_title(const char *title)
    {
        if (!m_initialized)
            return;
        m_title = title;
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

    inline bool is_resizable()
    {
        return m_resizeable;
    }

    inline void set_resizeable(bool t)
    {
        m_resizeable = t;
    }

    inline math::ivec2 get_position()
    {
        return m_screenPos;
    }

    inline std::string get_title()
    {
        return m_title;
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