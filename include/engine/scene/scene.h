/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SCENE_H
#define SCENE_H

#include <engine/scene/mesh.h>
#include <engine/scene/camera.h>
#include <engine/scene/light.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class Scene : public Object3D
{
private:
    Camera *m_activeCamera;

    std::vector<Camera *> m_cameras;
    std::vector<Mesh *> m_meshes;
    std::vector<Light *> m_lights;

    Vec3 m_ambientColor{0.7f, 0.5f, 0.5f};
    float m_ambientIntensity{0.2f};

    // SSAO
    bool m_ssao{true};
    float m_occRadius{0.5};
    float m_occBias{0.025};
    float m_ssaoBlurKernel{4};

    // FOG
    bool m_fog{true};
    Vec3 m_fogColor{0.2f, 0.2f, 0.2f};
    float m_fogIntensity{0.25f};
    float m_fogExponent{1.0f};

    inline void classify_object(Object3D *obj)
    {
        switch (obj->get_type())
        {
        case MESH:
            m_meshes.push_back((Mesh *)obj);
            break;
        case CAMERA:
            m_cameras.push_back((Camera *)obj);
            break;
        case LIGHT:
            // m_light = (Light *)obj;
            m_lights.push_back((Light *)obj);
            break;
        }
        for (auto child : obj->get_children())
            classify_object(child);
    }

public:
    Scene(Camera *cam) : m_activeCamera(cam) { add_child(cam); };
    ~Scene()
    {
        delete m_activeCamera;
        m_cameras.clear();
        m_meshes.clear();
    };
    inline void add(Object3D *obj)
    {
        add_child(obj);
    }
    inline void add_child(Object3D *obj)
    {
        classify_object(obj);
        Object3D::add_child(obj);
        isDirty = true;
    }

    inline Camera *const get_active_camera() const { return m_activeCamera; }
    inline const std::vector<Mesh *> get_meshes() const { return m_meshes; }
    inline const std::vector<Camera *> get_cameras() const { return m_cameras; }
    inline const std::vector<Light *> get_lights() const { return m_lights; }

    inline void set_ambient_color(Vec3 c) { m_ambientColor = c; }
    inline Vec3 get_ambient_color() const { return m_ambientColor; }

    inline void set_ambient_intensity(float i) { m_ambientIntensity = i; }
    inline float get_ambient_intensity() const { return m_ambientIntensity; }

    inline void enable_fog(bool op) { m_fog = op; }
    inline bool is_fog_enabled() const { return m_fog; }

    inline void set_fog_color(Vec3 c) { m_fogColor = c; }
    inline Vec3 get_fog_color() const { return m_fogColor; }

    inline void set_fog_intensity(float i) { m_fogIntensity = i; }
    inline float get_fog_intensity() const { return m_fogIntensity; }

    inline void enable_ssao(bool op) { m_ssao = op; }
    inline bool is_ssao_enabled() const { return m_ssao; }

    inline void set_ssao_radius(float i) { m_occRadius = i; }
    inline float get_ssao_radius() const { return m_occRadius; }

    inline void set_ssao_bias(float i) { m_occBias = i; }
    inline float get_ssao_bias() { return m_occBias; }
};

VULKAN_ENGINE_NAMESPACE_END
#endif