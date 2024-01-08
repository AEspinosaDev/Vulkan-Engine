#ifndef VK_SCENE
#define VK_SCENE

#include "vk_mesh.h"
#include "vk_camera.h"

namespace vke
{

    class Scene : public Object3D
    {
    private:
        Camera *m_activeCamera;

        std::vector<Camera *> m_cameras;
        std::vector<Mesh *> m_meshes;

        // std::vector<Light*> m_lights;
        // For now, just ONE light
        // Light* m_light

        bool m_fog{true};
        glm::vec3 m_fogColor{0.8f, 0.8f, 0.8f};
        float m_fogIntensity{2.0f};
        float m_fogExponent{1.0f};

        glm::vec3 ambientColor;
        glm::vec3 lightPosition;
        glm::vec3 lightColor;

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
                //
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
        // inline const std::vector<Mesh *> get_lights() const { return m_meshes; }

        inline void set_fog_active(bool op) { m_fog = op; }
        inline bool is_fog_active() const { return m_fog; }

        inline void set_fog_color(glm::vec3 c) { m_fogColor = c; }
        inline glm::vec3 get_fog_color() const { return m_fogColor; }

        inline void set_fog_intensity(float i) { m_fogIntensity = i; }
        inline float get_fog_intensity() const { return m_fogIntensity; }
    };

}
#endif