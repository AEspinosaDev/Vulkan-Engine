/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <engine/common.h>
#include <engine/core/scene/object3D.h>
#include <engine/core/windows/window.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Tools {

struct KeyMappings {
    int moveLeft     = GLFW_KEY_A;
    int moveRight    = GLFW_KEY_D;
    int moveForward  = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp       = GLFW_KEY_E;
    int moveDown     = GLFW_KEY_Q;

    int mouseLeft   = GLFW_MOUSE_BUTTON_1;
    int mouseMiddle = GLFW_MOUSE_BUTTON_2;
    int mouseRight  = GLFW_MOUSE_BUTTON_3;

    int reset = GLFW_KEY_R;
};

class Controller
{
  protected:
    Core::Object3D* m_objPtr;
    Core::IWindow*  m_windowPtr;

    float                  m_speed;
    ControllerMovementType m_type;
    KeyMappings            m_mappings;

    enum KeyActions
    {
        PRESS   = GLFW_PRESS,
        RELEASE = GLFW_RELEASE,
        REPEAT  = GLFW_REPEAT,
    };

    // MOUSE
    float m_mouseSensitivity;
    float m_mouseDeltaX;
    float m_mouseDeltaY;
    float m_mouseLastX;
    float m_mouseLastY;
    bool  m_firstMouse;
    bool  m_isMouseLeftPressed;
    bool  m_isMouseMiddlePressed;
    bool  m_isMouseRightPressed;

    Vec3 m_orbitalCenter;

    Core::Transform m_initialState;

    bool m_enabled{true};

  public:
    Controller(Core::Object3D*        obj,
               Core::IWindow*         window,
               ControllerMovementType m  = ControllerMovementType::WASD,
               KeyMappings            km = KeyMappings{})
        : m_objPtr(obj)
        , m_windowPtr(window)
        , m_type(m)
        , m_speed(5.0f)
        , m_mouseSensitivity(0.4f)
        , m_mouseDeltaX(.0f)
        , m_mouseDeltaY(.0f)
        , m_mouseLastX(.0f)
        , m_mouseLastY(0.0f)
        , m_firstMouse(true)
        , m_isMouseLeftPressed(false)
        , m_isMouseMiddlePressed(false)
        , m_isMouseRightPressed(false)
        , m_initialState(obj->get_transform())
        , m_mappings(km)
        , m_orbitalCenter({0.0, 0.0, 0.0}) {
    }

    virtual inline void set_active(const bool s) {
        m_enabled = s;
    }
    virtual inline bool is_active() const {
        return m_enabled;
    }

    inline ControllerMovementType get_type() const {
        return m_type;
    }
    inline void set_type(ControllerMovementType t) {
        m_type = t;
    }
    inline float get_speed() const {
        return m_speed;
    }
    inline void set_speed(float s) {
        m_speed = s;
    }
    inline float get_mouse_sensitivity() const {
        return m_mouseSensitivity;
    }
    inline void set_mouse_sensitivity(float s) {
        m_mouseSensitivity = s;
    }
    inline Core::Object3D* get_object() const {
        return m_objPtr;
    }
    inline void set_object(Core::Object3D* obj) {
        m_objPtr = obj;
    }
    inline Core::IWindow* get_window() const {
        return m_windowPtr;
    }
    inline void set_window(Core::IWindow* w) {
        m_windowPtr = w;
    }
    inline Vec3 get_orbital_center() const {
        return m_orbitalCenter;
    }
    inline void set_orbital_center(Vec3 orbitalCenter) {
        m_orbitalCenter = orbitalCenter;
    }

    /*Not insert as GLFW callback!*/
    virtual void handle_keyboard(int key, int action, const float deltaTime);
    virtual void handle_mouse(float xpos, float ypos, bool constrainPitch = true);
    virtual void handle_mouse_scroll() { /*WIP*/
    }
};

} // namespace Tools

VULKAN_ENGINE_NAMESPACE_END
#endif