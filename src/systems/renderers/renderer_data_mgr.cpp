/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

    ////////////////////////////////////////////////////////////////////////////////////

    In this Renderer's module you will find:

    Implementation of functions focused on managing the uploading to the GPU and caching of data needed for
    rendering.

    ////////////////////////////////////////////////////////////////////////////////////
*/
#include <engine/systems/renderers/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {
void BaseRenderer::update_global_data(Core::Scene* const scene) {
    PROFILING_EVENT()
    /*
    CAMERA UNIFORMS LOAD
    */
    Core::Camera* camera = scene->get_active_camera();
    if (camera->is_dirty())
        camera->set_projection(m_window->get_extent().width, m_window->get_extent().height);
    Graphics::CameraUniforms camData;
    camData.view         = camera->get_view();
    camData.proj         = camera->get_projection();
    camData.viewProj     = camera->get_projection() * camera->get_view();
    camData.position     = Vec4(camera->get_position(), 0.0f);
    camData.screenExtent = {m_window->get_extent().width, m_window->get_extent().height};
    camData.nearPlane    = camera->get_near();
    camData.farPlane     = camera->get_far();

    m_frames[m_currentFrame].uniformBuffers[GLOBAL_LAYOUT].upload_data(&camData, sizeof(Graphics::CameraUniforms), 0);

    /*
    SCENE UNIFORMS LOAD
    */
    Graphics::SceneUniforms sceneParams;
    sceneParams.fogParams = {
        camera->get_near(), camera->get_far(), scene->get_fog_intensity(), scene->is_fog_enabled()};
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

    std::vector<Core::Light*> lights = scene->get_lights();
    if (lights.size() > VK_MAX_LIGHTS)
        std::sort(lights.begin(), lights.end(), [=](Core::Light* a, Core::Light* b) {
            return math::length(a->get_position() - camera->get_position()) <
                   math::length(b->get_position() - camera->get_position());
        });

    size_t lightIdx{0};
    for (Core::Light* l : lights)
    {
        if (l->is_active())
        {
            sceneParams.lightUniforms[lightIdx] = l->get_uniforms(camera->get_view());
            Mat4 depthProjectionMatrix =
                math::perspective(math::radians(l->get_shadow_fov()), 1.0f, l->get_shadow_near(), l->get_shadow_far());
            Mat4 depthViewMatrix = math::lookAt(l->get_position(), l->get_shadow_target(), Vec3(0, 1, 0));
            sceneParams.lightUniforms[lightIdx].viewProj = depthProjectionMatrix * depthViewMatrix;
            lightIdx++;
        }
        if (lightIdx >= VK_MAX_LIGHTS)
            break;
    }
    sceneParams.numLights = static_cast<int>(lights.size());

    m_frames[m_currentFrame].uniformBuffers[GLOBAL_LAYOUT].upload_data(
        &sceneParams,
        sizeof(Graphics::SceneUniforms),
        Graphics::Utils::pad_uniform_buffer_size(sizeof(Graphics::CameraUniforms), m_device.get_GPU()));

    /*
    SKYBOX MESH AND TEXTURE UPLOAD
    */
    setup_skybox(scene);
}
void BaseRenderer::update_object_data(Core::Scene* const scene) {
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
                float distance =
                    glm::distance(scene->get_active_camera()->get_position(), blendMeshes[i]->get_position());
                sorted[distance] = blendMeshes[i];
            }

            // SECOND = TRANSPARENT OBJECTS SORTED FROM NEAR TO FAR
            for (std::map<float, Core::Mesh*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
            {
                meshes.push_back(it->second);
            }
            Core::set_meshes(scene, meshes);
        }

        unsigned int mesh_idx = 0;
        for (Core::Mesh* m : scene->get_meshes())
        {
            if (m) // If mesh exists
            {
                if (m->is_active() &&              // Check if is active
                    m->get_num_geometries() > 0 && // Check if has geometry
                    m->get_bounding_volume()->is_on_frustrum(
                        scene->get_active_camera()->get_frustrum())) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset =
                        m_frames[m_currentFrame].uniformBuffers[OBJECT_LAYOUT].strideSize * mesh_idx;

                    Graphics::ObjectUniforms objectData;
                    objectData.model        = m->get_model_matrix();
                    objectData.otherParams1 = {
                        m->is_affected_by_fog(), m->get_recive_shadows(), m->get_cast_shadows(), false};
                    objectData.otherParams2 = {m->is_selected(), m->get_bounding_volume()->center};
                    m_frames[m_currentFrame].uniformBuffers[1].upload_data(
                        &objectData, sizeof(Graphics::ObjectUniforms), objectOffset);

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        // Object vertex buffer setup
                        Core::Geometry* g = m->get_geometry(i);
                        upload_geometry_data(g);

                        // Object material setup
                        Core::IMaterial* mat = m->get_material(g->get_material_ID());
                        if (!mat)
                            mat = Core::IMaterial::DEBUG_MATERIAL;
                        if (mat)
                        {
                            auto textures = mat->get_textures();
                            for (auto pair : textures)
                            {
                                Core::ITexture* texture = pair.second;
                                upload_texture_image(texture);
                            }
                        }

                        // ObjectUniforms materialData;
                        Graphics::MaterialUniforms materialData = mat->get_uniforms();
                        m_frames[m_currentFrame].uniformBuffers[OBJECT_LAYOUT].upload_data(
                            &materialData,
                            sizeof(Graphics::MaterialUniforms),
                            objectOffset + Graphics::Utils::pad_uniform_buffer_size(
                                               sizeof(Graphics::MaterialUniforms), m_device.get_GPU()));
                    }
                }
            }
            mesh_idx++;
        }
    }
}

void BaseRenderer::upload_texture_image(Core::ITexture* const t) {
    if (t && t->loaded_on_CPU())
    {
        if (!t->loaded_on_GPU())
        {
            void* imgCache{nullptr};
            t->get_image_cache(imgCache);
            m_device.upload_texture_image(
                imgCache, t->get_bytes_per_pixel(), get_image(t), t->get_settings().useMipmaps);
        }
    }
}
void BaseRenderer::destroy_texture_image(Core::ITexture* const t) {
    if (t)
        get_image(t)->cleanup();
}
void BaseRenderer::upload_geometry_data(Core::Geometry* const g) {
    PROFILING_EVENT()
    Graphics::VertexArrays* rd = get_VAO(g);
    if (!rd->loadedOnGPU)
    {
        const Core::GeometricData* gd = g->get_geometric_data();

        size_t vboSize  = sizeof(gd->vertexData[0]) * gd->vertexData.size();
        size_t iboSize  = sizeof(gd->vertexIndex[0]) * gd->vertexIndex.size();
        rd->indexCount  = gd->vertexIndex.size();
        rd->vertexCount = gd->vertexIndex.size();

        m_device.upload_vertex_arrays(*rd, vboSize, gd->vertexData.data(), iboSize, gd->vertexIndex.data());
    }

    /*
    TO DO

    Upload vulkn RT ACCELERAITON STRUCTURES

    */
}

void BaseRenderer::destroy_geometry_data(Core::Geometry* const g) {

    Graphics::VertexArrays* rd = get_VAO(g);
    if (rd->loadedOnGPU)
    {
        rd->vbo.cleanup();
        if (rd->indexCount > 0)
            rd->ibo.cleanup();

        rd->loadedOnGPU = false;
    }
}
void BaseRenderer::setup_skybox(Core::Scene* const scene) {
    Core::Skybox* const skybox = scene->get_skybox();
    if (skybox)
    {
        if (skybox->update_enviroment())
        {
            upload_geometry_data(skybox->get_box());
            Core::TextureHDR* envMap = skybox->get_enviroment_map();
            if (envMap && envMap->loaded_on_CPU())
            {
                if (!envMap->loaded_on_GPU())
                {
                    void* imgCache{nullptr};
                    envMap->get_image_cache(imgCache);
                    m_device.upload_texture_image(imgCache, envMap->get_bytes_per_pixel(), get_image(envMap), false);
                }
                // Create Panorama converter pass
                if (m_renderPipeline.panoramaConverterPass)
                { // If already exists
                    m_renderPipeline.panoramaConverterPass->cleanup();
                    m_renderPipeline.irradianceComputePass->cleanup();
                    m_renderPipeline.panoramaConverterPass->clean_framebuffer();
                    m_renderPipeline.irradianceComputePass->clean_framebuffer();
                }
                m_renderPipeline.panoramaConverterPass =
                    new Core::PanoramaConverterPass(&m_device,
                                                    envMap->get_settings().format,
                                                    {envMap->get_size().height, envMap->get_size().height},
                                                    m_vignette);
                m_renderPipeline.panoramaConverterPass->setup(m_frames);
                m_renderPipeline.panoramaConverterPass->update_uniforms(m_currentFrame, scene);
                // Create Irradiance converter pass
                m_renderPipeline.irradianceComputePass = new Core::IrrandianceComputePass(
                    &m_device,
                    envMap->get_settings().format,
                    {m_settings.irradianceResolution, m_settings.irradianceResolution});
                m_renderPipeline.irradianceComputePass->setup(m_frames);
                m_renderPipeline.irradianceComputePass->update_uniforms(m_currentFrame, scene);
                m_renderPipeline.irradianceComputePass->connect_env_cubemap(
                    m_renderPipeline.panoramaConverterPass->get_attachments()[0].image);
            }
        }
    }
}
void BaseRenderer::init_resources() {

    // Setup frames
    m_frames.resize(static_cast<uint32_t>(m_settings.bufferingType));
    for (size_t i = 0; i < m_frames.size(); i++)
        m_frames[i].init(m_device.get_handle(), m_device.get_GPU(), m_device.get_swapchain().get_surface(), i);
    for (size_t i = 0; i < m_frames.size(); i++)
    {
        // Global Buffer
        Graphics::Buffer globalBuffer;
        const size_t     globalStrideSize =
            (Graphics::Utils::pad_uniform_buffer_size(sizeof(Graphics::CameraUniforms), m_device.get_GPU()) +
             Graphics::Utils::pad_uniform_buffer_size(sizeof(Graphics::SceneUniforms), m_device.get_GPU()));
        m_device.create_buffer(globalBuffer,
                               globalStrideSize,
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VMA_MEMORY_USAGE_CPU_TO_GPU,
                               (uint32_t)globalStrideSize);
        m_frames[i].uniformBuffers.push_back(globalBuffer);

        // Object Buffer
        Graphics::Buffer objectBuffer;
        const size_t     objectStrideSize =
            (Graphics::Utils::pad_uniform_buffer_size(sizeof(Graphics::ObjectUniforms), m_device.get_GPU()) +
             Graphics::Utils::pad_uniform_buffer_size(sizeof(Graphics::MaterialUniforms), m_device.get_GPU()));
        m_device.create_buffer(objectBuffer,
                               VK_MAX_OBJECTS * objectStrideSize,
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VMA_MEMORY_USAGE_CPU_TO_GPU,
                               (uint32_t)objectStrideSize);
        m_frames[i].uniformBuffers.push_back(objectBuffer);
    }

    // Setup vignette
    m_vignette = new Core::Mesh();
    m_vignette->push_geometry(Core::Geometry::create_quad());
    upload_geometry_data(m_vignette->get_geometry());

    // Setup fallback texture
    unsigned char texture_data[1] = {0};
    Core::Texture::FALLBACK_TEX   = new Core::Texture(texture_data, {1, 1, 1}, 4);
    Core::Texture::FALLBACK_TEX->set_use_mipmaps(false);
    void* imgCache{nullptr};
    Core::Texture::FALLBACK_TEX->get_image_cache(imgCache);
    m_device.upload_texture_image(
        imgCache, Core::Texture::FALLBACK_TEX->get_bytes_per_pixel(), get_image(Core::Texture::FALLBACK_TEX), false);
}

void BaseRenderer::clean_Resources() {
    for (size_t i = 0; i < m_frames.size(); i++)
    {
        for (Graphics::Buffer& buffer : m_frames[i].uniformBuffers)
        {
            buffer.cleanup();
        }
    }

    destroy_geometry_data(m_vignette->get_geometry());

    get_image(Core::Texture::FALLBACK_TEX)->cleanup();
}
} // namespace Systems

VULKAN_ENGINE_NAMESPACE_END