#pragma once
#include "vk_object3D.h"

namespace vkeng
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
        float m_speed;

        bool perspective;

        inline void set_view()
        {
            m_view = glm::lookAt(m_transform.position, m_transform.position - m_transform.forward, m_transform.up);
        }
        inline void update_vectors(float pitch, float yaw)
        { // Pitch/Yaw setup
            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            m_transform.forward =-glm::normalize(direction);
            m_transform.rotation =acos(direction);
            set_view();
        }

    public:
        Camera(glm::vec3 p = glm::vec3(0.0f, 1.0f, 8.0f), glm::vec3 f = glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : 
        Object3D(p), m_yaw(-90.0f), m_pitch(0.0f), m_speed(0.1f), m_fov(45.0f), m_near(.1f), m_far(100.0f)
        {
            set_view();
        }

        inline void set_field_of_view(float fov) { m_fov = fov; }
        inline float get_field_of_view() { return m_fov; }
        inline void set_projection(int width, int height) { m_proj = glm::perspective(glm::radians(m_fov), (float)width / (float)height, m_near, m_far); }
        inline glm::mat4 get_projection() { return m_proj; }
        inline glm::mat4 get_view() { return m_view; }
        inline float get_far() { return m_far; }
        inline void set_far(float f) { m_far = f; }
        inline float get_near() { return m_near; }
        inline void set_near(float n) { m_near = n; }
        inline void set_speed(float n) { m_speed = n; }
        inline float get_speed() { return m_speed; }

        inline void set_position(glm::vec3 p)
        {
            Object3D::set_position(p);
            update_vectors(m_pitch, m_yaw);
        }

        void update();

    };

}