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

        Transform m_initialState;

    public:
        CameraController(Camera *cam, MovementType m = WASD) : m_camera(cam), m_type(m),m_cameraSpeed(2.0f),m_initialState(cam->get_transform()) {}

        inline MovementType get_type() { return m_type; }
        inline float get_speed() { return m_cameraSpeed; }
        inline void set_speed(float s) { m_cameraSpeed=s; }

        virtual void handle_keyboard(GLFWwindow *window, const float deltaTime);
        virtual void handle_mouse(float xoffset, float yoffset, bool constrainPitch = true);
        virtual void handle_mouseS_scroll()
        { /*WIP*/
        }
    };

}
#endif