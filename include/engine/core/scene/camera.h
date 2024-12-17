/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CAMERA_H
#define CAMERA_H

#include <engine/core/scene/object3D.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

struct Face {
    Vec3  normal   = {0.f, 1.f, 0.f}; // unit vector
    float distance = 0.f;             // Distance with origin

    Face() = default;

    Face(const Vec3& p1, const Vec3& norm)
        : normal(math::normalize(norm))
        , distance(math::dot(normal, p1)) {
    }
    Face(const float dist, const Vec3& norm)
        : normal(math::normalize(norm))
        , distance(dist) {
    }

    float get_signed_distance(const Vec3& point) const {
        return math::dot(normal, point) - distance;
    }
};

struct Frustum {
    Face topFace;
    Face bottomFace;

    Face rightFace;
    Face leftFace;

    Face farFace;
    Face nearFace;
};

class Camera : public Object3D
{

  private:
    Mat4 m_view;
    Mat4 m_proj;

    Frustum m_frustrum;

    float m_fov;
    float m_near;
    float m_far;
    float m_zoom;
    float m_aspect;

    bool m_perspective{true};
    bool m_frustrumCulling{true};

    static int m_instanceCount;

  public:
    Camera(Vec3 p = Vec3(0.0f, 1.0f, 8.0f), Vec3 f = Vec3(0.0f, 0.0f, 1.0f), Vec3 up = Vec3(0.0f, 1.0f, 0.0f))
        : Object3D("Camera #" + std::to_string(Camera::m_instanceCount), p, ObjectType::CAMERA)
        , m_fov(45.0f)
        , m_near(.1f)
        , m_far(100.0f) {
        set_rotation({-90, 0, 0}, true);
        Camera::m_instanceCount++;
    }

    inline void set_field_of_view(float fov) {
        m_fov   = fov;
        isDirty = true;
    }
    inline float get_field_of_view() const {
        return m_fov;
    }
    inline void set_projection(int width, int height) {
        m_aspect = (float)width / (float)height;
        // m_proj   = math::perspective(math::radians(m_fov), m_aspect, m_near, m_far);
        m_proj = math::perspectiveRH_ZO(math::radians(m_fov), m_aspect, m_near, m_far);
        m_proj[1][1] *= -1; // Because Vulkan
    }
    inline Mat4 get_projection() const {
        return m_proj;
    }
    inline Mat4 get_view() {
        if (isDirty)
        {
            m_view = math::lookAt(m_transform.position, m_transform.position + m_transform.forward, m_transform.up);
            set_frustum();
            isDirty = false;
        }
        return m_view;
    }
    inline float get_far() const {
        return m_far;
    }
    inline void set_far(float f) {
        m_far   = f;
        isDirty = true;
    }
    inline float get_near() const {
        return m_near;
    }
    inline void set_near(float n) {
        m_near  = n;
        isDirty = true;
    }

    void set_frustum();

    inline Frustum get_frustrum() {
        if (isDirty)
        {
            set_frustum();
        }
        return m_frustrum;
    }

    inline void enable_frustrum_culling(bool op) {
        m_frustrumCulling = op;
    }
    inline bool get_frustrum_culling() const {
        return m_frustrumCulling;
    }
};
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
#endif // VK_CAMERA_H