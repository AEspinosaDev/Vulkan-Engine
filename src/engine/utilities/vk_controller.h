#ifndef VK_CONTROLLER_H
#define VK_CONTROLLER_H

#include "../internal/vk_core.h"
#include "../vk_camera.h"

namespace vke
{
    enum MovementType
    {
        ORBITAL,
        WASD,
    };

    class CameraController
    {
    protected:
        Camera *m_camera;
        float m_cameraSpeed;
        MovementType m_type;

        //MOUSE
        float m_mouseSensitivity;
        float m_mouseDeltaX;
        float m_mouseDeltaY;
        float m_mouseLastX;
        float m_mouseLastY;
        bool m_firstMouse;
        bool m_isMouseLeftPressed;
        bool m_isMouseMiddlePressed;
        bool m_isMouseRightPressed;

        Transform m_initialState;

    public:
        CameraController(Camera *cam, MovementType m = WASD) : m_camera(cam), m_type(m), m_cameraSpeed(100.0f),
                                                               m_mouseSensitivity(0.4f), m_mouseDeltaX(.0f), m_mouseDeltaY(.0f),
                                                               m_mouseLastX(.0f), m_mouseLastY(0.0f), m_firstMouse(true),
                                                               m_isMouseLeftPressed(false), m_isMouseMiddlePressed(false), m_isMouseRightPressed(false),
                                                               m_initialState(cam->get_transform()) {}

        inline MovementType get_type() { return m_type; }
        inline float get_speed() const { return m_cameraSpeed; }
        inline void set_speed(float s) { m_cameraSpeed = s; }
        inline float get_mouse_sensitivity() const { return m_mouseSensitivity; }
        inline void set_mouse_sensitivity(float s) { m_mouseSensitivity = s; }

        virtual void handle_keyboard(GLFWwindow *window, const float deltaTime);
        virtual void handle_mouse(GLFWwindow *window, float xpos, float ypos, bool constrainPitch = true);
        virtual void handle_mouse_scroll()
        { /*WIP*/
        }
    };

}
#endif