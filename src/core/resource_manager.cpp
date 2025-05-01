#include <engine/core/resource_manager.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

std::vector<Core::ITexture*> ResourceManager::textureResources;
Core::Texture*               ResourceManager::FALLBACK_TEXTURE = nullptr;
Core::Texture*               ResourceManager::FALLBACK_CUBEMAP = nullptr;
Mat4                         ResourceManager::prevViewProj;

void ResourceManager::init_basic_resources(Graphics::Device* const device) {

    // Setup vignette
    Core::BasePass::vignette = Core::Geometry::create_quad();
    upload_geometry_data(device, Core::BasePass::vignette);

    // Setup fallback texture
    if (!FALLBACK_TEXTURE) // If not user set
    {
        unsigned char texture_data[1] = {0};
        FALLBACK_TEXTURE              = new Core::Texture(texture_data, {1, 1, 1}, 4);
        FALLBACK_TEXTURE->set_use_mipmaps(false);
    }
    upload_texture_data(device, FALLBACK_TEXTURE);
    if (!FALLBACK_CUBEMAP) // If not user set
    {
        unsigned char cube_data[6] = {0, 0, 0, 0, 0, 0};
        FALLBACK_CUBEMAP           = new Core::Texture(cube_data, {1, 1, 1}, 4);
        FALLBACK_CUBEMAP->set_use_mipmaps(false);
        FALLBACK_CUBEMAP->set_type(TextureTypeFlagBits::TEXTURE_CUBE);
    }
    upload_texture_data(device, FALLBACK_CUBEMAP);
}

void ResourceManager::clean_basic_resources() {
    destroy_geometry_data(Core::BasePass::vignette);
    destroy_texture_data(FALLBACK_TEXTURE);
    destroy_texture_data(FALLBACK_CUBEMAP);
    for (Core::ITexture* texture : textureResources)
        destroy_texture_data(texture);
}
void ResourceManager::update_global_data(Graphics::Device* const device,
                                         Graphics::Frame* const  currentFrame,
                                         Core::Scene* const      scene,
                                         Extent2D                displayExtent,
                                         bool                    jitterCamera) {
    PROFILING_EVENT()
    /*
    CAMERA UNIFORMS LOAD
    */
    Core::Camera* camera = scene->get_active_camera();
    if (camera->is_dirty())
        camera->set_projection(displayExtent.width, displayExtent.height);
    Graphics::CameraUniforms camData;
    camData.view     = camera->get_view();
    camData.proj     = camera->get_projection();
    camData.viewProj = camera->get_projection() * camera->get_view();
    /*Inversed*/
    camData.invView     = math::inverse(camData.view);
    camData.invProj     = math::inverse(camData.proj);
    camData.invViewProj = math::inverse(camData.viewProj);
    /*Windowed*/
    const glm::mat4 W{
        displayExtent.width / 2.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        displayExtent.height / 2.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        displayExtent.width / 2.0f,
        displayExtent.height / 2.0f,
        0.0f,
        1.0f,
    };
    camData.unormProj     = W * camData.proj;
    camData.prevViewProj  = prevViewProj;
    static int frameIndex = 0;
    frameIndex            = (frameIndex + 1) % 16;
    camData.jitter        = jitterCamera ? Utils::get_halton_jitter(frameIndex, displayExtent.width, displayExtent.height) : Vec2{0.0, 0.0};

    /*Other intersting Camera Data*/
    camData.position     = Vec4(camera->get_position(), 0.0f);
    camData.screenExtent = {displayExtent.width, displayExtent.height};
    camData.nearPlane    = camera->get_near();
    camData.farPlane     = camera->get_far();

    currentFrame->uniformBuffers[GLOBAL_LAYOUT].upload_data(&camData, sizeof(Graphics::CameraUniforms), 0);

    /*
    SCENE UNIFORMS LOAD
    */

    Graphics::SceneUniforms sceneParams;
    sceneParams.fogParams       = {camera->get_near(), camera->get_far(), scene->get_fog_intensity(), scene->is_fog_enabled()};
    sceneParams.fogColorAndSSAO = Vec4(scene->get_fog_color(), 0.0f);
    sceneParams.SSAOtype        = 0;
    sceneParams.emphasizeAO     = false;
    sceneParams.ambientColor    = Vec4(scene->get_ambient_color(), scene->get_ambient_intensity());
    sceneParams.useIBL          = scene->use_IBL();
    if (scene->get_skybox()) // If skybox
    {
        sceneParams.envRotation        = scene->get_skybox()->get_rotation();
        sceneParams.envColorMultiplier = scene->get_skybox()->get_intensity();
    }
    sceneParams.time = 0.0;

    /*Limits*/
    // UPDATE AABB OF SCENE FOR VOXELIZATION PURPOSES
    scene->update_AABB();
    AABB aabb            = scene->get_AABB();
    sceneParams.maxCoord = Vec4(aabb.maxCoords, 1.0f);
    sceneParams.minCoord = Vec4(aabb.minCoords, 1.0f);

    std::vector<Core::Light*> lights = scene->get_lights();
    if (lights.size() > ENGINE_MAX_LIGHTS)
        std::sort(lights.begin(), lights.end(), [=](Core::Light* a, Core::Light* b) {
            return math::length(a->get_position() - camera->get_position()) < math::length(b->get_position() - camera->get_position());
        });

    size_t lightIdx{0};
    for (Core::Light* l : lights)
    {
        if (l->is_active())
        {
            /*Check if is directIonal and behaves as SUN*/
            if (l->get_light_type() == LightType::DIRECTIONAL && scene->get_skybox())
            {
                DirectionalLight* dirL = static_cast<DirectionalLight*>(l);
                if (dirL->use_as_sun())
                {
                    dirL->set_direction(
                        -DirectionalLight::get_sun_direction(scene->get_skybox()->get_sky_settings().sunElevationDeg, -sceneParams.envRotation - 90.0f));
                }
            }

            sceneParams.lightUniforms[lightIdx] = l->get_uniforms(camera->get_view());
            Mat4 depthProjectionMatrix          = math::perspective(math::radians(l->get_shadow_fov()), 1.0f, l->get_shadow_near(), l->get_shadow_far());
            Mat4 depthViewMatrix                = math::lookAt(l->get_position(), l->get_shadow_target(), Vec3(0, 1, 0));
            sceneParams.lightUniforms[lightIdx].viewProj = depthProjectionMatrix * depthViewMatrix;
            lightIdx++;
        }
        if (lightIdx >= ENGINE_MAX_LIGHTS)
            break;
    }
    sceneParams.numLights = static_cast<int>(lights.size());

    currentFrame->uniformBuffers[GLOBAL_LAYOUT].upload_data(
        &sceneParams, sizeof(Graphics::SceneUniforms), device->pad_uniform_buffer_size(sizeof(Graphics::CameraUniforms)));
}
void ResourceManager::update_object_data(Graphics::Device* const device,
                                         Graphics::Frame* const  currentFrame,
                                         Core::Scene* const      scene,
                                         Extent2D                displayExtent,
                                         bool                    enableRT) {

    PROFILING_EVENT()

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {
        std::vector<Core::Mesh*> meshes;
        std::vector<Core::Mesh*> blendMeshes;

        for (Core::Mesh* m : scene->get_meshes())
        {
            if (m->get_material())
                m->get_material()->get_parameters().blending ? blendMeshes.push_back(m) : meshes.push_back(m);
        }

        // Calculate distance
        if (!blendMeshes.empty())
        {

            std::map<float, Core::Mesh*> sorted;
            for (unsigned int i = 0; i < blendMeshes.size(); i++)
            {
                float distance   = glm::distance(scene->get_active_camera()->get_position(), blendMeshes[i]->get_position());
                sorted[distance] = blendMeshes[i];
            }

            // SECOND = TRANSPARENT OBJECTS SORTED FROM NEAR TO FAR
            for (std::map<float, Core::Mesh*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
            {
                meshes.push_back(it->second);
            }
            Core::set_meshes(scene, meshes);
        }

        std::vector<Graphics::BLASInstance> BLASInstances; // RT Acceleration Structures per instanced mesh
        BLASInstances.reserve(scene->get_meshes().size());
        unsigned int mesh_idx = 0;
        for (Core::Mesh* m : scene->get_meshes())
        {
            if (m) // If mesh exists
            {
                if (m->is_active() &&                                                                     // Check if is active
                    m->get_num_geometries() > 0 &&                                                        // Check if has geometry
                    m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum())) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset = currentFrame->uniformBuffers[OBJECT_LAYOUT].strideSize * mesh_idx;

                    Graphics::ObjectUniforms objectData;
                    objectData.model        = m->get_model_matrix();
                    objectData.otherParams1 = {m->affected_by_fog(), m->receive_shadows(), m->cast_shadows(), 0.1f};
                    objectData.otherParams2 = {m->is_selected(), m->get_bounding_volume()->center};
                    currentFrame->uniformBuffers[OBJECT_LAYOUT].upload_data(&objectData, sizeof(Graphics::ObjectUniforms), objectOffset);

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        // Object vertex buffer setup
                        Core::Geometry* g = m->get_geometry(i);
                        upload_geometry_data(device, g, enableRT && m->ray_hittable());
                        // Add BLASS to instances list
                        if (enableRT && m->ray_hittable() && get_BLAS(g)->handle)
                            BLASInstances.push_back({*get_BLAS(g), m->get_model_matrix()});

                        // Object material setup
                        Core::IMaterial* mat = m->get_material(g->get_material_ID());
                        if (!mat)
                        {
                            m->push_material(Core::IMaterial::debugMaterial);
                        }
                        mat = m->get_material(g->get_material_ID());
                        if (mat)
                        {
                            auto textures = mat->get_textures();
                            for (auto pair : textures)
                            {
                                Core::ITexture* texture = pair.second;
                                upload_texture_data(device, texture);
                            }
                        }

                        // ObjectUniforms materialData;
                        Graphics::MaterialUniforms materialData = mat->get_uniforms();
                        currentFrame->uniformBuffers[OBJECT_LAYOUT].upload_data(
                            &materialData,
                            sizeof(Graphics::MaterialUniforms),
                            objectOffset + device->pad_uniform_buffer_size(sizeof(Graphics::MaterialUniforms)));
                    }
                }
            }
            mesh_idx++;
        }
        // CREATE TOP LEVEL (STATIC) ACCELERATION STRUCTURE
        if (enableRT)
        {
            Graphics::TLAS* accel = get_TLAS(scene);
            if (!accel->handle)
                device->upload_TLAS(*accel, BLASInstances);
            // Update Acceleration Structure if change in objects
            if (accel->instances < BLASInstances.size() || scene->update_AS())
            {
                device->wait_idle();
                device->upload_TLAS(*accel, BLASInstances);
                scene->update_AS(false);
            }
        }
    }
}
void ResourceManager::upload_texture_data(Graphics::Device* const device, Core::ITexture* const t) {
    if (t && t->loaded_on_CPU())
    {
        if (!t->loaded_on_GPU())
        {
            Graphics::ImageConfig   config        = {};
            Graphics::SamplerConfig samplerConfig = {};
            Core::TextureSettings   textSettings  = t->get_settings();
            config.viewType                       = textSettings.type;
            config.format                         = textSettings.format;
            config.mipLevels                      = textSettings.maxMipLevel;
            config.useMipmaps                     = textSettings.useMipmaps;
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

void ResourceManager::destroy_texture_data(Core::ITexture* const t) {
    if (t)
        get_image(t)->cleanup();
}
void ResourceManager::upload_geometry_data(Graphics::Device* const device, Core::Geometry* const g, bool createAccelStructure) {
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
void ResourceManager::destroy_geometry_data(Core::Geometry* const g) {
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
void ResourceManager::upload_skybox_data(Graphics::Device* const device, Core::Skybox* const sky) {
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
                config.useMipmaps                     = textSettings.useMipmaps;
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

void ResourceManager::clean_scene(Core::Scene* const scene) {
    if (scene)
    {
        for (Core::Mesh* m : scene->get_meshes())
        {
            for (size_t i = 0; i < m->get_num_geometries(); i++)
            {
                Core::Geometry* g = m->get_geometry(i);
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
        }
        if (scene->get_skybox())
        {
            destroy_geometry_data(scene->get_skybox()->get_box());
            destroy_texture_data(scene->get_skybox()->get_enviroment_map());
        }
        get_TLAS(scene)->cleanup();
    }
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END;
