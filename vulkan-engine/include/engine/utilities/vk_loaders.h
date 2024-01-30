#ifndef VK_LOADERS
#define VK_LOADERS

#include <unordered_map>
#include <tiny_obj_loader.h>
#include "../scene_objects/vk_mesh.h"

namespace vke
{
    // Meshes
    namespace loaders
    {
        bool load_OBJ(Mesh *const mesh, bool overrideGeometry, const std::string fileName, bool importMaterials = false, bool calculateTangents = false);
        void compute_tangents_gram_smidt(std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
    };
    // TO DO
    // namespace FBXLoader
    // {
    // };

}

#endif