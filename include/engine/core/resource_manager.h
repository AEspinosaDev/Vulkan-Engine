/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/

#ifndef RES_MANAGER_H
#define RES_MANAGER_H

#include <engine/common.h>
#include <engine/core/materials/material.h>
#include <engine/core/renderpasses/irradiance_compute_pass.h>
#include <engine/core/renderpasses/panorama_conversion_pass.h>
#include <engine/core/textures/texture.h>
#include <engine/core/textures/textureLDR.h>
#include <engine/core/windows/window.h>
#include <engine/core/windows/windowGLFW.h>

#include <engine/graphics/device.h>

#include <engine/tools/loaders.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/*
Used internally by the Renderer Class
--------------
Static utility class that handles resource creation, uploading to the GPU and cleanup.
*/
class ResourceManager
{
  public:
    /*
    Auxiliar passes
    */
    static Core::PanoramaConverterPass*  panoramaConverterPass;
    static Core::IrrandianceComputePass* irradianceComputePass;
    /*
    Fallback Resources
    */
    static Core::Texture* FALLBACK_TEXTURE;
    static Core::Texture* FALLBACK_CUBEMAP;
    static Core::Texture* BLUE_NOISE_TEXTURE;
    /*
    Vignette for RTT
    */
    static Core::Mesh* VIGNETTE;
    /*
    Creates and initiates basic rendering resources such as fallback textures and a vignette
    */
    static void init_basic_resources(Graphics::Device* const device);
    static void clean_basic_resources();

    /*
    Global descriptor layouts uniforms buffer upload to GPU
    */
    static void update_global_data(Graphics::Device* const device,
                                   Graphics::Frame* const  currentFrame,
                                   Core::Scene* const      scene,
                                   Core::IWindow* const    window);
    /*
    Object descriptor layouts uniforms buffer upload to GPU
    */
    static void update_object_data(Graphics::Device* const device,
                                   Graphics::Frame* const  currentFrame,
                                   Core::Scene* const      scene,
                                   Core::IWindow* const    window,
                                   bool                    enableRT);
    /*
    Initialize and setup texture IMAGE
    */
    static void upload_texture_data(Graphics::Device* const device, Core::ITexture* const t);
    static void destroy_texture_data(Core::ITexture* const t);
    /*
    Upload geometry vertex buffers to the GPU
    */
    static void
    upload_geometry_data(Graphics::Device* const device, Core::Geometry* const g, bool createAccelStructure = true);
    static void destroy_geometry_data(Core::Geometry* const g);
    /*
    Setup skybox
    */
    static void setup_skybox(Graphics::Device* const device, Core::Scene* const scene);
    static void generate_skybox_maps(Graphics::Frame* const currentFrame, Core::Scene* const scene);
    /*
    Scene cleanup
    */
    static void clean_scene(Core::Scene* const scene);
};

} // namespace Systems

VULKAN_ENGINE_NAMESPACE_END;

#endif // VK_GEOMETRY_H