/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef LOADERS_H
#define LOADERS_H

#include <chrono>
#include <stb_image.h>
#include <thread>
#include <tiny_obj_loader.h>
#include <tinyply.h>
#include <tinyxml2.h>
#include <unordered_map>

#include <engine/core/materials/phong.h>
#include <engine/core/materials/physically_based.h>
#include <engine/core/materials/unlit.h>
#include <engine/core/scene/mesh.h>
#include <engine/core/scene/scene.h>
#include <engine/core/textures/textureHDR.h>
#include <engine/core/textures/textureLDR.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

// Load functions for several mesh and image files
namespace Tools::Loaders {
void load_OBJ(Core::Mesh* const mesh,
              const std::string fileName,
              bool              importMaterials   = false,
              bool              calculateTangents = false,
              bool              overrideGeometry  = false);

void load_PLY(Core::Mesh* const mesh,
              const std::string fileName,
              bool              preload           = true,
              bool              verbose           = false,
              bool              calculateTangents = false,
              bool              overrideGeometry  = false);
/*
Generic loader. It automatically parses the file and find the needed loader for the file extension. Can be called
asynchronously
*/
void load_3D_file(Core::Mesh* const mesh,
                  const std::string fileName,
                  bool              asynCall         = true,
                  bool              overrideGeometry = false);
/*
Use on .hair files.
*/
void load_hair(Core::Mesh* const mesh, const char* fileName);
/*
Load image texture
*/
void load_texture(Core::ITexture*   texture,
                  const std::string fileName,
                  TextureFormatType textureFormat = TEXTURE_FORMAT_SRGB,
                  bool              asyncCall     = true);
/*
Load .png file.
 */
void load_PNG(Core::Texture* const texture,
              const std::string    fileName,
              TextureFormatType    textureFormat = TEXTURE_FORMAT_SRGB);
/*
Load .hrd
*/
void load_HDRi(Core::TextureHDR* const texture, const std::string fileName);
/*
Load texture as 3D image. It will require and image with all the layers defined. The larger of their extent properties
will be used for computing the depth if no depthy input is given. PNG or JPEG available.
*/
void load_3D_texture(Core::ITexture* const texture,
                     const std::string     fileName,
                     uint16_t              depth         = 0,
                     TextureFormatType     textureFormat = TEXTURE_FORMAT_SRGB);

/*Loads and save a scene from XML file*/
class SceneLoader
{
    // load_mesh();
    // load_light();
    // load_camera();

  public:
    SceneLoader() {
    }
    void load_scene(Core::Scene* const scene, const std::string fileName, bool async = true);
    void save_scene(const std::string fileName) {
    }
};

}; // namespace Tools::Loaders

VULKAN_ENGINE_NAMESPACE_END

#endif