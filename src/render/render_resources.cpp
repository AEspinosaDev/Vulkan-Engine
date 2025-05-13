#include <engine/render/render_resources.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {


void Resources::init_shared_resources(const ptr<Graphics::Device>& device) {

    // Setup vignette
    vignette = Core::Geometry::create_quad();
    upload_geometry_data(device, vignette);

    // Setup fallback texture
    if (!fallbackTexture2D) // If not user set
    {
        unsigned char texture_data[1] = {0};
        fallbackTexture2D              = new Core::TextureLDR(texture_data, {1, 1, 1}, 4);
        fallbackTexture2D->set_use_mipmaps(false);
    }
    upload_texture_data(device, fallbackTexture2D);
    if (!fallbackCubeMap) // If not user set
    {
        unsigned char cube_data[6] = {0, 0, 0, 0, 0, 0};
        fallbackCubeMap           = new Core::TextureLDR(cube_data, {1, 1, 1}, 4);
        fallbackCubeMap->set_use_mipmaps(false);
        fallbackCubeMap->set_type(TextureTypeFlagBits::TEXTURE_CUBE);
    }
    upload_texture_data(device, fallbackCubeMap);
}

void Resources::clean_shared_resources() {
    destroy_geometry_data(vignette);
    destroy_texture_data(fallbackTexture2D);
    destroy_texture_data(fallbackCubeMap);
    for (Core::ITexture* texture : sharedTextures)
        destroy_texture_data(texture);
}

void Resources::upload_texture_data(const ptr<Graphics::Device>& device, Core::ITexture* const t) {
    if (t && t->loaded_on_CPU())
    {
        if (!t->loaded_on_GPU())
        {
            Graphics::ImageConfig   config        = {};
            Graphics::SamplerConfig samplerConfig = {};
            Core::TextureSettings   textSettings  = t->get_settings();
            config.viewType                       = textSettings.type;
            config.format                         = textSettings.format;
            config.mipLevels                      = textSettings.useMipmaps ? textSettings.maxMipLevel : 1;
            samplerConfig.anysotropicFilter       = textSettings.anisotropicFilter;
            samplerConfig.filters                 = textSettings.filter;
            samplerConfig.maxLod                  = textSettings.maxMipLevel;
            samplerConfig.minLod                  = textSettings.minMipLevel;
            samplerConfig.samplerAddressMode      = textSettings.adressMode;

            void* imgCache{nullptr};
            t->get_image_cache(imgCache);
            device->upload_texture_image(*get_image(t), config, samplerConfig, imgCache, t->get_bytes_per_pixel());
        }
    }
}

void Resources::destroy_texture_data(Core::ITexture* const t) {
    if (t)
        get_image(t)->cleanup();
}
void Resources::upload_geometry_data(const ptr<Graphics::Device>& device, Core::Geometry* const g, bool createAccelStructure) {
    PROFILING_EVENT()
    /*
    VERTEX ARRAYS
    */
    Graphics::VertexArrays* rd = get_VAO(g);
    if (!rd->loadedOnGPU)
    {
        const Core::GeometricData& gd        = g->get_properties();
        size_t                     vboSize   = sizeof(gd.vertexData[0]) * gd.vertexData.size();
        size_t                     iboSize   = sizeof(gd.vertexIndex[0]) * gd.vertexIndex.size();
        size_t                     voxelSize = sizeof(gd.voxelData[0]) * gd.voxelData.size();
        rd->indexCount                       = gd.vertexIndex.size();
        rd->vertexCount                      = gd.vertexData.size();
        rd->voxelCount                       = gd.voxelData.size();

        device->upload_vertex_arrays(*rd, vboSize, gd.vertexData.data(), iboSize, gd.vertexIndex.data(), voxelSize, gd.voxelData.data());
    }
    /*
    ACCELERATION STRUCTURE
    */
    if (createAccelStructure)
    {
        Graphics::BLAS* accel = get_BLAS(g);
        if (!accel->handle)
            device->upload_BLAS(*accel, *get_VAO(g));
    }
}
void Resources::destroy_geometry_data(Core::Geometry* const g) {
    Graphics::VertexArrays* rd = get_VAO(g);
    if (rd->loadedOnGPU)
    {
        rd->vbo.cleanup();
        if (rd->indexCount > 0)
            rd->ibo.cleanup();
        if (rd->voxelCount > 0)
            rd->voxelBuffer.cleanup();

        rd->loadedOnGPU = false;
        get_BLAS(g)->cleanup();
    }
}
void Resources::upload_skybox_data(const ptr<Graphics::Device>& device, Core::Skybox* const sky) {
    if (sky)
    {
        upload_geometry_data(device, sky->get_box());
        Core::TextureHDR* envMap = sky->get_enviroment_map();
        if (envMap && envMap->loaded_on_CPU())
        {
            if (!envMap->loaded_on_GPU())
            {
                Graphics::ImageConfig   config        = {};
                Graphics::SamplerConfig samplerConfig = {};
                Core::TextureSettings   textSettings  = envMap->get_settings();
                config.format                         = textSettings.format;
                samplerConfig.anysotropicFilter       = textSettings.anisotropicFilter;
                samplerConfig.filters                 = textSettings.filter;
                samplerConfig.samplerAddressMode      = textSettings.adressMode;

                void* imgCache{nullptr};
                envMap->get_image_cache(imgCache);
                device->upload_texture_image(*get_image(envMap), config, samplerConfig, imgCache, envMap->get_bytes_per_pixel());
            }
        }
    }
}

void Resources::clean_scene(Core::Scene* const scene) {
    if (scene)
    {
        for (Core::Mesh* m : scene->get_meshes())
        {

            Core::Geometry* g = m->get_geometry();
            destroy_geometry_data(g);

            Core::IMaterial* mat = m->get_material(g->get_material_ID());
            if (mat)
            {
                auto textures = mat->get_textures();
                for (auto pair : textures)
                {
                    Core::ITexture* texture = pair.second;
                    destroy_texture_data(texture);
                }
            }
        }
        if (scene->get_skybox())
        {
            destroy_geometry_data(scene->get_skybox()->get_box());
            destroy_texture_data(scene->get_skybox()->get_enviroment_map());
        }
        get_TLAS(scene)->cleanup();
    }
}

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END;
