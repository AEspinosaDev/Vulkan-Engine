/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef RENDER_RESOURCES
#define RENDER_RESOURCES
#include <engine/graphics/device.h>

#include <engine/core/geometries/geometry.h>
#include <engine/core/scene/scene.h>
#include <engine/core/textures/texture_template.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {
/*
Global Render Resources Data and Utilities
*/
struct Resources {
    Core::Geometry*              vignette;
    std::vector<Core::ITexture*> sharedTextures;
    Core::ITexture*              fallbackTexture2D;
    Core::ITexture*              fallbackCubeMap;
    Mat4                         prevViewProj;

    void init_shared_resources(const ptr<Graphics::Device>& device);
    void clean_shared_resources();

    static void upload_texture_data(const ptr<Graphics::Device>& device, Core::ITexture* const t);
    static void destroy_texture_data(Core::ITexture* const t);
    static void upload_geometry_data(const ptr<Graphics::Device>& device, Core::Geometry* const g, bool createAccelStructure = true);
    static void destroy_geometry_data(Core::Geometry* const g);
    static void upload_skybox_data(const ptr<Graphics::Device>& device, Core::Skybox* const sky);
    static void clean_scene(Core::Scene* const scene);
};

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END
#endif