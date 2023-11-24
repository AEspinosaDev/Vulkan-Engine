#ifndef VK_SCENE_H
#define VK_SCENE_H

#include "vk_mesh.h"
#include "vk_camera.h"

namespace vke
{

    class Scene : public Object3D
    {
    private:
        std::vector<Mesh *> m_meshes;
        Camera *m_currentCamera;
        // std::vector<Light*> m_lights;
        // delete m_parent;

    public:
        Scene(Camera *cam) : m_currentCamera(cam) { add_child(cam); };
        ~Scene();
        inline void add(Object3D *obj)
        {
            switch (obj->get_type())
            {
            case MESH:
                m_meshes.push_back(obj);
                break;
            case LIGHT:
                //
                break;
            }
            add_child(obj);
        }
    };

}
#endif