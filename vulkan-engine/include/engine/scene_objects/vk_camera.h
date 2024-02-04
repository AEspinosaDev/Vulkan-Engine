#ifndef VK_CAMERA
#define VK_CAMERA

#include <engine/vk_object3D.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class Camera : public Object3D
{

private:
    Mat4 m_view;
    Mat4 m_proj;

    float m_fov;
    float m_near;
    float m_far;
    float m_zoom;

    bool perspective;

    static int m_cameraCount;

public:
    Camera(Vec3 p = Vec3(0.0f, 1.0f, 8.0f), Vec3 f = Vec3(0.0f, 0.0f, 1.0f), Vec3 up = Vec3(0.0f, 1.0f, 0.0f)) : Object3D("Camera #" + std::to_string(Camera::m_cameraCount), p, CAMERA), m_fov(45.0f), m_near(.1f), m_far(100.0f)
    {
        set_rotation({-90, 0, 0});
        Camera::m_cameraCount++;
    }

    inline void set_field_of_view(float fov)
    {
        m_fov = fov;
        isDirty = true;
    }
    inline float get_field_of_view() const { return m_fov; }
    inline void set_projection(int width, int height)
    {
        m_proj = glm::perspective(glm::radians(m_fov), (float)width / (float)height, m_near, m_far);
        m_proj[1][1] *= -1; // Because Vulkan
    }
    inline Mat4 get_projection() const { return m_proj; }
    inline Mat4 get_view() { return get_model_matrix(); }
    inline float get_far() const { return m_far; }
    inline void set_far(float f)
    {
        m_far = f;
        isDirty = true;
    }
    inline float get_near() const { return m_near; }
    inline void set_near(float n)
    {
        m_near = n;
        isDirty = true;
    }

    inline Mat4 get_model_matrix()
    {
        if (isDirty)
        {
            m_view = glm::lookAt(m_transform.position, m_transform.position + m_transform.forward, m_transform.up);
            isDirty = false;
        }
        return m_view;
    }
};

VULKAN_ENGINE_NAMESPACE_END
#endif // VK_CAMERA_H