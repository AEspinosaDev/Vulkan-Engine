#ifndef VK_CAMERA
#define VK_CAMERA

#include "../vk_object3D.h"

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
        float m_zoom;

        bool perspective;

        static int m_cameraCount;

    public:
        Camera(glm::vec3 p = glm::vec3(0.0f, 1.0f, 8.0f), glm::vec3 f = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : Object3D("Camera #" + std::to_string(Camera::m_cameraCount), p, CAMERA), m_fov(45.0f), m_near(.1f), m_far(100.0f)
        {
            set_rotation({-90, 0, 0});
            Camera::m_cameraCount++;
        }

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

        inline glm::mat4 get_model_matrix()
        {
            if (isDirty)
            {
                m_view = glm::lookAt(m_transform.position, m_transform.position + m_transform.forward, m_transform.up);
                isDirty = false;
            }
            return m_view;
        }
    };
    
}
#endif // VK_CAMERA_H