/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/

#ifndef RES_MANAGER_H
#define RES_MANAGER_H

#include <engine/common.h>
#include <engine/core/materials/material.h>
#include <engine/core/textures/texture.h>
#include <engine/core/textures/texture_template.h>
#include <engine/core/windows/window.h>
#include <engine/core/windows/windowGLFW.h>

#include <engine/graphics/device.h>
#include <engine/render/GPU_resource_pool.h>

#include <engine/tools/loaders.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {

// class RenderViewBuilder
// {
//     std::string m_global_UBO_Key;
//     std::string m_object_UBO_Key;

// public:
//     inline void set_keys( const std::string& globalUBO_key, const std::string& objectUBO_key ) {
//         m_global_UBO_Key = globalUBO_key;
//         m_object_UBO_Key = objectUBO_key;
//     }

class GPUSceneBuilder
{
public:
    // Build a GPU view of the scene (uploads all data to the GPU)
    void build( const ptr<Graphics::Device>& device,
                Graphics::Frame* const       currentFrame,
                Core::Scene* const           scene,
                Extent2D                     displayExtent,
                bool                         raytracingEnabled,
                bool                         temporalFiltering );
    /*
    Uploads scene's skybox resources (cube mesh and panorama texture)
    */
    void build_skybox_data( const ptr<Graphics::Device>& device, Core::Skybox* const sky );
    // Destroys the GPU view of the scene
    void destroy( Core::Scene* const scene );

private:
    /*
    Global descriptor layouts uniforms buffer upload to GPU
    */
    void update_global_data( const ptr<Graphics::Device>& device,
                             Graphics::Frame* const       currentFrame,
                             Core::Scene* const           scene,
                             Extent2D                     displayExtent,
                             bool                         jitterCamera );
    /*
    Object descriptor layouts uniforms buffer upload to GPU
    */
    void update_object_data( const ptr<Graphics::Device>& device,
                             Graphics::Frame* const       currentFrame,
                             Core::Scene* const           scene,
                             Extent2D                     displayExtent,
                             bool                         enableRT );
   
    /*
    Scene cleanup
    */
    void clean_scene( Core::Scene* const scene );
};

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END;

#endif // VK_GEOMETRY_H