/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef LOADERS_H
#define LOADERS_H

#include <chrono>
#include <thread>
#include <tiny_obj_loader.h>
#include <tinyply.h>
#include <stb_image.h>
#include <unordered_map>

#include <engine/core/textures/texture.h>
#include <engine/core/textures/textureLDR.h>
#include <engine/core/scene/mesh.h>

VULKAN_ENGINE_NAMESPACE_BEGIN


// Load functions for several mesh and image files
namespace Tools::Loaders
{
void load_OBJ(Core::Mesh *const mesh, const std::string fileName, bool importMaterials = false,
              bool calculateTangents = false, bool overrideGeometry = false);

void load_PLY(Core::Mesh *const mesh, const std::string fileName, bool preload = true, bool verbose = false,
              bool calculateTangents = false, bool overrideGeometry = false);
/*
Generic loader. It automatically parses the file and find the needed loader for the file extension. Can be called
asynchronously
*/
void load_3D_file(Core::Mesh *const mesh, const std::string fileName, bool asynCall = true,
                  bool overrideGeometry = false);
/*
Use on .hair files.
*/
void load_hair(Core::Mesh *const mesh, const char *fileName);
/*
Load image texture
*/
void load_texture(Core::TextureBase *const texture, const std::string fileName, bool asyncCall = true);
/*
Load .png file.
 */
void load_PNG(Core::TextureLDR *const texture, const std::string fileName);

/*
Load .hrd
*/
void load_HDRi(Core::TextureBase *const texture, const std::string fileName);
}; // namespace Loaders

VULKAN_ENGINE_NAMESPACE_END

#endif