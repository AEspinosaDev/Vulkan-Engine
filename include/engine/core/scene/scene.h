/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SCENE_H
#define SCENE_H

#include <engine/core/scene/camera.h>
#include <engine/core/scene/light.h>
#include <engine/core/scene/mesh.h>
#include <engine/core/scene/skybox.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class Scene : public Object3D
{
  private:
    Camera*              m_activeCamera;
    std::vector<Camera*> m_cameras;
    std::vector<Mesh*>   m_meshes;
    std::vector<Light*>  m_lights;
    Skybox*              m_skybox = nullptr;

    bool           m_updateAccel = false;
    Graphics::TLAS m_accel       = {};
    // Graphics::TLAS m_dynamicAccel = {};

    // ENVIROMENT & FOG
    bool  m_useIBL           = false;
    Vec3  m_ambientColor     = {0.7f, 0.5f, 0.5f}; // Fallback if no skybox
    float m_ambientIntensity = 0.2f;
    bool  m_fog              = true;
    Vec3  m_fogColor         = {0.2f, 0.2f, 0.2f};
    float m_fogIntensity     = 20.0f;
    float m_fogExponent      = 1.0f;
    // BVOL
    AABB m_volume = (this);

    inline void classify_object(Object3D* obj) {
        switch (obj->get_type())
        {
        case ObjectType::MESH:
            m_meshes.push_back(static_cast<Mesh*>(obj));
            break;
        case ObjectType::CAMERA:
            m_cameras.push_back(static_cast<Camera*>(obj));
            m_activeCamera = static_cast<Camera*>(obj);
            break;
        case ObjectType::LIGHT:
            m_lights.push_back(static_cast<Light*>(obj));
            break;
        }
        for (auto child : obj->get_children())
            classify_object(child);
    }

    friend void            set_meshes(Scene* const scene, std::vector<Mesh*> meshes);
    friend Graphics::TLAS* get_TLAS(Scene* const scene);

  public:
    Scene(Camera* cam)
        : m_activeCamera(cam) {
        add_child(cam);
    };
    Scene()
        : m_activeCamera(nullptr) {};
    ~Scene() {
        delete m_activeCamera;
        delete m_skybox;

        for (Mesh* m : m_meshes)
        {
            delete m;
        }
        for (Camera* c : m_cameras)
        {
            delete c;
        }
        for (Light* l : m_lights)
        {
            delete l;
        }

        m_cameras.clear();
        m_meshes.clear();
        m_lights.clear();
    };
    inline void add(Object3D* obj) {
        add_child(obj);
    }
    inline void add_child(Object3D* obj) {
        classify_object(obj);
        Object3D::add_child(obj);
        isDirty = true;
    }

    inline Camera* const get_active_camera() const {
        return m_activeCamera;
    }
    inline const std::vector<Mesh*> get_meshes() const {
        return m_meshes;
    }
    inline const std::vector<Camera*> get_cameras() const {
        return m_cameras;
    }
    inline const std::vector<Light*> get_lights() const {
        return m_lights;
    }
    inline void set_skybox(Skybox* skb) {
        m_skybox = skb;
        m_useIBL = true;
    }
    inline Skybox* const get_skybox() const {
        return m_skybox;
    }
    inline void set_ambient_color(Vec3 c) {
        m_ambientColor = c;
    }
    inline Vec3 get_ambient_color() const {
        return m_ambientColor;
    }

    inline void set_ambient_intensity(float i) {
        m_ambientIntensity = i;
    }
    inline float get_ambient_intensity() const {
        return m_ambientIntensity;
    }

    inline void enable_fog(bool op) {
        m_fog = op;
    }
    inline bool is_fog_enabled() const {
        return m_fog;
    }

    inline void set_fog_color(Vec3 c) {
        m_fogColor = c;
    }
    inline Vec3 get_fog_color() const {
        return m_fogColor;
    }

    inline void set_fog_intensity(float i) {
        m_fogIntensity = i;
    }
    inline float get_fog_intensity() const {
        return m_fogIntensity;
    }
    inline bool use_IBL() const {
        return m_useIBL;
    }
    inline void use_IBL(bool p) {
        m_useIBL = p;
    }
    /*
   Query if Acceleration Structure is dynamic. That means that AS will update the positions of the meshes.
   */
    inline bool dynamic_AS() const {
        return m_accel.dynamic;
    }
    /*
    Set if Acceleration Structure is dynamic. That means that AS will update the positions of the meshes.
    */
    inline void dynamic_AS(bool op) {
        m_accel.dynamic = op;
    }
    inline bool update_AS() const {
        return m_updateAccel;
    }
    inline void update_AS(bool op) {
        m_updateAccel = op;
    }

    /*
    Setup axis-aligned bounding volume for the entire scene. This object might be necessary for some functionalities,
    sush as voxelization of the scene.
    */
    void        setup_AABB();
    inline AABB get_AABB() const {
        return m_volume;
    }
};

void set_meshes(Scene* const scene, std::vector<Mesh*> meshes);

Graphics::TLAS* get_TLAS(Scene* const scene);
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
#endif