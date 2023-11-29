#ifndef VK_CAMERA_H
#define VK_CAMERA_H

#include "vk_object3D.h"

namespace vke
{

    class Camera : public Object3D
    {

    private:
        glm::mat4 m_view;
        glm::mat4 m_proj;

        float m_fov;
        float m_near;
        float m_far;
        float m_yaw;
        float m_pitch;
        float m_zoom;

        bool perspective;

    public:
        Camera(glm::vec3 p = glm::vec3(0.0f, 1.0f, 8.0f), glm::vec3 f = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : Object3D(p, OTHER), m_yaw(-90.0f), m_pitch(0.0f), m_fov(45.0f), m_near(.1f), m_far(100.0f) {}

        inline void set_field_of_view(float fov) { m_fov = fov; }
        inline float get_field_of_view() { return m_fov; }
        inline void set_projection(int width, int height)
        {
            m_proj = glm::perspective(glm::radians(m_fov), (float)width / (float)height, m_near, m_far);
            m_proj[1][1] *= -1; // Because Vulkan
        }
        inline glm::mat4 get_projection() { return m_proj; }
        inline glm::mat4 get_view() { return get_model_matrix(); }
        inline float get_far() { return m_far; }
        inline void set_far(float f) { m_far = f; }
        inline float get_near() { return m_near; }
        inline void set_near(float n) { m_near = n; }
        inline float get_pitch(){ return m_pitch;}
        inline void set_pitch(float p){m_pitch=p;}
        inline void set_yaw(float p){m_yaw=p;}
        inline float get_yaw(){ return m_yaw;}
        inline glm::mat4 get_model_matrix()
        {
            if (isDirty)
            {
                m_view = glm::lookAt(m_transform.position, m_transform.position + m_transform.forward, m_transform.up);
                isDirty = false;
            }
            return m_view;
        }
        inline void update_vectors()
        {
            // Pitch/Yaw setup
            glm::vec3 direction;
            direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            direction.y = sin(glm::radians(m_pitch));
            direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_transform.forward = -glm::normalize(direction);
            m_transform.rotation = acos(direction);
            isDirty = true;
        }
    };
}
#endif // VK_CAMERA_H