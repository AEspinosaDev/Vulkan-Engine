#ifndef VK_LOADERS
#define VK_LOADERS

#include <unordered_map>
#include <tiny_obj_loader.h>
#include "../scene_objects/vk_mesh.h"

namespace vke
{
    // Meshes
    namespace OBJLoader
    {
        bool load_mesh(Mesh *const mesh, bool overrideGeometry, const std::string fileName, bool importMaterials = false, bool calculateTangents = false);
    };
    // TO DO
    // namespace FBXLoader
    // {
    // };
   
}

#endif