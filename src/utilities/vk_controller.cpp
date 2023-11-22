#include "vk_controller.h"
namespace vkeng
{
    void CameraController::handle_keyboard(GLFWwindow *window, const float deltaTime)
    {
        if (m_type == WASD)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() + m_camera->get_transform().forward * m_cameraSpeed);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() - m_camera->get_transform().forward * m_cameraSpeed);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() - glm::normalize(glm::cross(m_camera->get_transform().forward, m_camera->get_transform().up)) * m_cameraSpeed);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() + glm::normalize(glm::cross(m_camera->get_transform().forward, m_camera->get_transform().up)) * m_cameraSpeed);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() - glm::normalize(m_camera->get_transform().up) * m_cameraSpeed);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                m_camera->set_position(m_camera->get_position() + glm::normalize(m_camera->get_transform().up) * m_cameraSpeed);

            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            {
                m_camera->set_transform(m_initialState);
            }
        }
        if (m_type == ORBITAL)
        {
        }
    }

    void CameraController::handle_mouse(float xoffset, float yoffset, bool constrainPitch)
    {
        // // Pressing
        // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
        // {
        //     r->m_UtilParams.isMouseLeftPressed = true;

        //     if (r->m_UtilParams.firstMouse)
        //     {
        //         r->m_UtilParams.mouselastX = xpos;
        //         r->m_UtilParams.mouselastY = ypos;
        //         r->m_UtilParams.firstMouse = false;
        //     }

        //     float xoffset = xpos - r->m_UtilParams.mouselastX;
        //     float yoffset = r->m_UtilParams.mouselastY - ypos; // reversed since y-coordinates go from bottom to top

        //     r->m_UtilParams.mouselastX = xpos;
        //     r->m_UtilParams.mouselastY = ypos;

        //     if (r->m_UtilParams.canControl)
        //         r->m_ActiveController->handleMouse(r->m_CurrentScene->getActiveCamera(), xoffset, yoffset);
        // }

        // if (m_type == WASD)
        // {
        //     xoffset *= camera->mouseSensitivity;
        //     yoffset *= camera->mouseSensitivity;

        //     camera->yaw += xoffset;
        //     camera->pitch += yoffset;

        //     if (constrainPitch)
        //     {
        //         if (camera->pitch > 89.0f)
        //             camera->pitch = 89.0f;
        //         if (camera->pitch < -89.0f)
        //             camera->pitch = -89.0f;
        //     }

        //     // update Front, Right and Up Vectors using the updated Euler angles
        //     camera->updateCameraVectors(camera->pitch, camera->yaw);
    // }
    // if (m_type == ORBITAL)
    // {
    // }
}
}