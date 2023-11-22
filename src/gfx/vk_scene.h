#pragma once
#include "vk_mesh.h"
#include "vk_camera.h"

namespace vkeng
{

    class Scene : public Object3D
    {
    private:
        std::vector<Mesh *> m_meshes;
        Camera *m_currentCamera;
        // std::vector<Light*> m_lights;

    public:
        Scene(/* args */);
        ~Scene();
    };

    Scene::Scene(/* args */)
    {
    }

    Scene::~Scene()
    {
    }

}