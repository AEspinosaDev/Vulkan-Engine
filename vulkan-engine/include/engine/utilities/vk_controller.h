#ifndef VK_CONTROLLER
#define VK_CONTROLLER

#include <engine/vk_common.h>
#include <engine/vk_object3D.h>
VULKAN_ENGINE_NAMESPACE_BEGIN

struct KeyMappings
{
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp = GLFW_KEY_E;
    int moveDown = GLFW_KEY_Q;

    int mouseLeft = GLFW_MOUSE_BUTTON_1;
    int mouseMiddle = GLFW_MOUSE_BUTTON_2;
    int mouseRight = GLFW_MOUSE_BUTTON_3;

    int reset = GLFW_KEY_R;
};

class Controller
{
protected:
    Object3D *m_objPtr;
    float m_speed;
    ControllerMovementType m_type;
    KeyMappings m_mappings;

    enum KeyActions
    {
        PRESS = GLFW_PRESS,
        RELEASE = GLFW_RELEASE,
        REPEAT = GLFW_REPEAT,
    };

    // MOUSE
    float m_mouseSensitivity;
    float m_mouseDeltaX;
    float m_mouseDeltaY;
    float m_mouseLastX;
    float m_mouseLastY;
    bool m_firstMouse;
    bool m_isMouseLeftPressed;
    bool m_isMouseMiddlePressed;
    bool m_isMouseRightPressed;

    Vec3 m_orbitalCenter;

    Transform m_initialState;

    bool m_enabled{true};

public:
    Controller(Object3D *obj, ControllerMovementType m = WASD, KeyMappings km = KeyMappings{}) : m_objPtr(obj), m_type(m), m_speed(5.0f),
                                                                                                 m_mouseSensitivity(0.4f), m_mouseDeltaX(.0f), m_mouseDeltaY(.0f),
                                                                                                 m_mouseLastX(.0f), m_mouseLastY(0.0f), m_firstMouse(true),
                                                                                                 m_isMouseLeftPressed(false), m_isMouseMiddlePressed(false), m_isMouseRightPressed(false),
                                                                                                 m_initialState(obj->get_transform()), m_mappings(km),
                                                                                                 m_orbitalCenter({0.0, 0.0, 0.0}) {}

    virtual inline void set_active(const bool s)
    {
        m_enabled = s;
    }
    virtual inline bool is_active() const { return m_enabled; }

    inline ControllerMovementType get_type() { return m_type; }
    inline float get_speed() const { return m_speed; }
    inline void set_speed(float s) { m_speed = s; }
    inline float get_mouse_sensitivity() const { return m_mouseSensitivity; }
    inline void set_mouse_sensitivity(float s) { m_mouseSensitivity = s; }
    inline Object3D *get_object() const { return m_objPtr; }
    inline void set_object(Object3D *obj) { m_objPtr = obj; }
    /*Not insert as GLFW callback!*/
    virtual void handle_keyboard(GLFWwindow *window, int key, int action, const float deltaTime);
    virtual void handle_mouse(GLFWwindow *window, float xpos, float ypos, bool constrainPitch = true);
    virtual void handle_mouse_scroll()
    { /*WIP*/
    }
};

VULKAN_ENGINE_NAMESPACE_END
#endif