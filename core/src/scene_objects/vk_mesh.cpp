#include "engine/scene_objects/vk_mesh.h"
#include "engine/utilities/vk_loaders.h"

namespace vke
{

    bool Mesh::load_file(const std::string fileName, bool overrideGeometry)
    {
        //TO DO: check for the termination of the file and call the specific loader
        //FOR NOW it only supports OBJ
        return OBJLoader::load_mesh(this,overrideGeometry, fileName);
    }
    Mesh *Mesh::clone() const
    {
        return new Mesh(this->m_geometry, this->m_material);
    }
}