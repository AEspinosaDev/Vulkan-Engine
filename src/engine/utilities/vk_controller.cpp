#include "vk_controller.h"
namespace vke
{
    void CameraController::handle_keyboard(GLFWwindow *window, const float deltaTime)
    {
        auto speed = m_cameraSpeed * deltaTime;

        if (m_type == WASD)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() + m_camera->get_transform().forward * speed);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() - m_camera->get_transform().forward * speed);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() - glm::normalize(glm::cross(m_camera->get_transform().forward, m_camera->get_transform().up)) * speed);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() + glm::normalize(glm::cross(m_camera->get_transform().forward, m_camera->get_transform().up)) * speed);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() - glm::normalize(m_camera->get_transform().up) * speed);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() + glm::normalize(m_camera->get_transform().up) * speed);

            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            {
                m_camera->set_transform(m_initialState);
                m_camera->set_pitch(0.0f);
                m_camera->set_yaw(0.0f);
            }
        }
        if (m_type == ORBITAL)
        {
        }
    }

    void CameraController::handle_mouse(GLFWwindow *window, float xpos, float ypos, bool constrainPitch)
    {
        // Pressing
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
        {
            m_isMouseLeftPressed = true;
            if (m_firstMouse)
            {
                m_mouseLastX = xpos;
                m_mouseLastY = ypos;
                m_firstMouse = false;
            }

            m_mouseDeltaX = xpos - m_mouseLastX;
            m_mouseDeltaY = ypos - m_mouseLastY;
            m_mouseLastX = xpos;
            m_mouseLastY = ypos;

            if (m_type == WASD)
            {
                m_mouseDeltaX *= m_mouseSensitivity;
                m_mouseDeltaY *= m_mouseSensitivity;

                m_camera->set_yaw(m_camera->get_yaw() + m_mouseDeltaX);
                m_camera->set_pitch(m_camera->get_pitch() + m_mouseDeltaY);

                if (constrainPitch)
                {
                    if (m_camera->get_pitch() > 89.0f)
                        m_camera->set_pitch(89.0f);
                    if (m_camera->get_pitch() < -89.0f)
                        m_camera->set_pitch(-89.0f);
                }

                m_camera->update_vectors();
            }
            if (m_type == ORBITAL)
            {
            }
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
        {
            m_isMouseMiddlePressed = true;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS)
        {
            m_isMouseRightPressed = true;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
        {
            m_firstMouse = true;
            m_isMouseLeftPressed = false;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
        {
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_RELEASE)
        {
            m_isMouseRightPressed = false;
        }
    }
}