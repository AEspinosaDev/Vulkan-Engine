#include "engine/scene_objects/vk_mesh.h"
#include "engine/utilities/vk_loaders.h"

namespace vke
{
    Geometry *Mesh::change_geometry(Geometry *g, size_t id)
    {
        if (m_geometry.size() < id + 1)
        {
            ERR_LOG("Not enough geometry slots");
            return nullptr;
        }
        Geometry *old_g = m_geometry[id];
        m_geometry[id] = g;
        return old_g;
    }
    Material *Mesh::change_material(Material *m, size_t id)
    {
        if (m_material.size() < id + 1)
        {
            ERR_LOG("Not enough material slots");
            return nullptr;
        }

        Material *old_m = m_material[id];
        m_material[id] = m;
        return old_m;
    }

    bool Mesh::load_file(const std::string fileName, bool overrideGeometry)
    {
        // TO DO: check for the termination of the file and call the specific loader
        // FOR NOW it only supports OBJ
        return OBJLoader::load_mesh(this, overrideGeometry, fileName);
    }
    Mesh *Mesh::clone() const
    {
        Mesh *mesh = new Mesh();
        for (auto m : m_material)
        {
            mesh->set_material(m);
        }
        for (auto g : m_geometry)
        {
            mesh->set_geometry(g);
        }
        return mesh;
    }
}