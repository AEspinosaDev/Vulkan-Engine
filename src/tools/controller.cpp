#include <engine/tools/controller.h>
VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Tools {
void Controller::handle_keyboard(int key, int action, const float deltaTime) {

    if (!m_enabled)
        return;

    void* windowHandle{nullptr};
    m_windowPtr->get_handle(windowHandle);
    GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);

    auto speed = m_speed * deltaTime;

    ///////////////////////
    // FORWARD AND BACWARD
    ///////////////////////
    if (glfwGetKey(window, m_mappings.moveForward) == PRESS)
        m_objPtr->set_position(m_objPtr->get_position() + m_objPtr->get_transform().forward * speed);
    if (glfwGetKey(window, m_mappings.moveBackward) == PRESS)
        m_objPtr->set_position(m_objPtr->get_position() - m_objPtr->get_transform().forward * speed);
    ///////////////////////
    // LEFT AND RIGHT
    ///////////////////////
    if (glfwGetKey(window, m_mappings.moveLeft) == PRESS)
    {
        Vec3 direction =
            glm::normalize(glm::cross(m_objPtr->get_transform().forward, m_objPtr->get_transform().up)) * speed;
        m_objPtr->set_position(m_objPtr->get_position() - direction);
        if (m_type == ControllerMovementType::ORBITAL)
            m_orbitalCenter -= direction;
    }
    if (glfwGetKey(window, m_mappings.moveRight) == PRESS)
    {
        Vec3 direction =
            glm::normalize(glm::cross(m_objPtr->get_transform().forward, m_objPtr->get_transform().up)) * speed;
        m_objPtr->set_position(m_objPtr->get_position() + direction);
        if (m_type == ControllerMovementType::ORBITAL)
            m_orbitalCenter += direction;
    }
    ///////////////////////
    // UP AND DOWN
    ///////////////////////
    if (glfwGetKey(window, m_mappings.moveDown) == PRESS)
    {
        Vec3 direction = glm::normalize(m_objPtr->get_transform().up) * speed;
        m_objPtr->set_position(m_objPtr->get_position() - direction);
        if (m_type == ControllerMovementType::ORBITAL)
            m_orbitalCenter -= direction;
    }
    if (glfwGetKey(window, m_mappings.moveUp) == PRESS)
    {
        Vec3 direction = glm::normalize(m_objPtr->get_transform().up) * speed;
        m_objPtr->set_position(m_objPtr->get_position() + direction);
        if (m_type == ControllerMovementType::ORBITAL)
            m_orbitalCenter += direction;
    }
    if (glfwGetKey(window, m_mappings.reset) == PRESS)
    {
        m_objPtr->set_transform(m_initialState);
        if (m_type == ControllerMovementType::ORBITAL)
            m_orbitalCenter = Vec3(0.0f);
    }
}

void Controller::handle_mouse(float xpos, float ypos, bool constrainPitch) {
    if (!m_enabled)
        return;

    void* windowHandle{nullptr};
    m_windowPtr->get_handle(windowHandle);
    GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);

    // Pressing
    if (glfwGetMouseButton(window, m_mappings.mouseLeft) == PRESS)
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
        m_mouseLastX  = xpos;
        m_mouseLastY  = ypos;

        m_mouseDeltaX *= m_mouseSensitivity;
        m_mouseDeltaY *= m_mouseSensitivity;

        float yaw   = m_objPtr->get_rotation(true).x + m_mouseDeltaX;
        float pitch = m_objPtr->get_rotation(true).y + m_mouseDeltaY;

        if (constrainPitch)
        {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;
        }

        m_objPtr->set_rotation({yaw, pitch, 0.0}, true);

        if (m_type == ControllerMovementType::ORBITAL)
        {

            float radius = glm::distance(m_objPtr->get_position(), m_orbitalCenter);
            float x      = m_orbitalCenter.x + radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
            float y      = m_orbitalCenter.y + radius * sin(glm::radians(pitch));
            float z      = m_orbitalCenter.z + radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

            m_objPtr->set_position({x, y, z});
        }
    }
    if (glfwGetMouseButton(window, m_mappings.mouseMiddle) == PRESS)
    {
        m_isMouseMiddlePressed = true;
    }
    if (glfwGetMouseButton(window, m_mappings.mouseRight) == PRESS)
    {
        m_isMouseRightPressed = true;
    }
    if (glfwGetMouseButton(window, m_mappings.mouseLeft) == RELEASE)
    {
        m_firstMouse         = true;
        m_isMouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, m_mappings.mouseMiddle) == RELEASE)
    {
    }
    if (glfwGetMouseButton(window, m_mappings.mouseRight) == RELEASE)
    {
        m_isMouseRightPressed = false;
    }
}
} // namespace Tools
VULKAN_ENGINE_NAMESPACE_END