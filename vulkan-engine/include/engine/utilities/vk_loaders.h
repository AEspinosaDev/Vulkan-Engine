#ifndef VK_LOADERS
#define VK_LOADERS

#include <unordered_map>
#include <thread>
#include <chrono>
#include <tiny_obj_loader.h>
#include <tinyply.h>
#include <engine/scene_objects/vk_mesh.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
// Meshes
namespace loaders
{
    void load_OBJ(Mesh *const mesh, const std::string fileName, bool importMaterials = false, bool calculateTangents = false, bool overrideGeometry = false);
    void load_PLY(Mesh *const mesh, const std::string fileName, bool preload = true, bool verbose = false, bool calculateTangents = false, bool overrideGeometry = false);
    void compute_tangents_gram_smidt(std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
    /*
    Generic loader. It automatically parses the file and find the needed loader for the file extension. Can be called asynchronously
    */
    void load_3D_file(Mesh *const mesh, const std::string fileName, bool asynCall = true, bool overrideGeometry = false);
  
};

VULKAN_ENGINE_NAMESPACE_END

#endif