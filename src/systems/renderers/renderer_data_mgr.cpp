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
namespace Systems
{
void Renderer::update_global_data(Core::Scene *const scene)
{
    PROFILING_EVENT()
    /*
    CAMERA UNIFORMS LOAD
    */
    Core::Camera *camera = scene->get_active_camera();
    if (camera->is_dirty())
        camera->set_projection(m_window->get_extent().width, m_window->get_extent().height);
    Graphics::CameraUniforms camData;
    camData.view = camera->get_view();
    camData.proj = camera->get_projection();
    camData.viewProj = camera->get_projection() * camera->get_view();
    camData.position = Vec4(camera->get_position(), 0.0f);
    camData.screenExtent = {m_window->get_extent().width, m_window->get_extent().height};

    m_context.frames[m_currentFrame].uniformBuffers[GLOBAL_LAYOUT].upload_data(m_context.memory, &camData,
                                                                               sizeof(Graphics::CameraUniforms), 0);

    /*
    SCENE UNIFORMS LOAD
    */
    Graphics::SceneUniforms sceneParams;
    sceneParams.fogParams = {camera->get_near(), camera->get_far(), scene->get_fog_intensity(),
                             scene->is_fog_enabled()};
    sceneParams.fogColorAndSSAO = Vec4(scene->get_fog_color(), 0.0f);
    sceneParams.SSAOtype = 0;
    sceneParams.emphasizeAO = false;
    sceneParams.ambientColor = Vec4(scene->get_ambient_color(), scene->get_ambient_intensity());

    std::vector<Core::Light *> lights = scene->get_lights();
    if (lights.size() > VK_MAX_LIGHTS)
        std::sort(lights.begin(), lights.end(), [=](Core::Light *a, Core::Light *b) {
            return math::length(a->get_position() - camera->get_position()) <
                   math::length(b->get_position() - camera->get_position());
        });

    size_t lightIdx{0};
    for (Core::Light *l : lights)
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

    m_context.frames[m_currentFrame].uniformBuffers[GLOBAL_LAYOUT].upload_data(
        m_context.memory, &sceneParams, sizeof(Graphics::SceneUniforms),
        Graphics::utils::pad_uniform_buffer_size(sizeof(Graphics::CameraUniforms), m_context.gpu));
}
void Renderer::update_object_data(Core::Scene *const scene)
{
    PROFILING_EVENT()

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {
        std::vector<Core::Mesh *> meshes;
        std::vector<Core::Mesh *> blendMeshes;

        for (Core::Mesh *m : scene->get_meshes())
        {
            if (m->get_material())
                m->get_material()->get_parameters().blending ? blendMeshes.push_back(m) : meshes.push_back(m);
        }

        // Calculate distance
        if (!blendMeshes.empty())
        {

            std::map<float, Core::Mesh *> sorted;
            for (unsigned int i = 0; i < blendMeshes.size(); i++)
            {
                float distance =
                    glm::distance(scene->get_active_camera()->get_position(), blendMeshes[i]->get_position());
                sorted[distance] = blendMeshes[i];
            }

            // SECOND = TRANSPARENT OBJECTS SORTED FROM NEAR TO FAR
            for (std::map<float, Core::Mesh *>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
            {
                meshes.push_back(it->second);
            }
            scene->set_meshes(meshes);
        }

        unsigned int mesh_idx = 0;
        for (Core::Mesh *m : scene->get_meshes())
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
                        m_context.frames[m_currentFrame].uniformBuffers[OBJECT_LAYOUT].strideSize * mesh_idx;

                    Graphics::ObjectUniforms objectData;
                    objectData.model = m->get_model_matrix();
                    objectData.otherParams1 = {m->is_affected_by_fog(), m->get_recive_shadows(), m->get_cast_shadows(),
                                               false};
                    objectData.otherParams2 = {m->is_selected(), 0.0, 0.0, 0.0};
                    m_context.frames[m_currentFrame].uniformBuffers[1].upload_data(
                        m_context.memory, &objectData, sizeof(Graphics::ObjectUniforms), objectOffset);

                    // void *meshData = nullptr;
                    // size_t size = 0;
                    // m->get_uniform_data(meshData, size);
                    // m_context.frames[m_currentFrame].uniformBuffers[OBJECT_LAYOUT].upload_data(
                    //     m_context.memory, meshData, size, objectOffset);
                    // free(meshData);

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        // Object vertex buffer setup
                        Core::Geometry *g = m->get_geometry(i);
                        upload_geometry_data(g);

                        // Object material setup
                        Core::Material *mat = m->get_material(g->get_material_ID());
                        if (mat)
                            upload_material_textures(mat);
                        else
                            upload_material_textures(Core::Material::DEBUG_MATERIAL);

                        // ObjectUniforms materialData;
                        Graphics::MaterialUniforms materialData = mat->get_uniforms();
                        m_context.frames[m_currentFrame].uniformBuffers[OBJECT_LAYOUT].upload_data(
                            m_context.memory, &materialData, sizeof(Graphics::MaterialUniforms),
                            objectOffset + Graphics::utils::pad_uniform_buffer_size(sizeof(Graphics::MaterialUniforms),
                                                                                    m_context.gpu));
                    }
                }
            }
            mesh_idx++;
        }
    }
}

void Renderer::upload_material_textures(Core::Material *const mat)
{
    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        Core::Texture *texture = pair.second;
        if (texture && texture->data_loaded())
        {
            if (!texture->is_buffer_loaded())
            {

                m_context.upload_texture_image(get_image(texture), texture->get_settings().useMipmaps);

                m_deletionQueue.push_function(
                    [=]() { get_image(texture)->cleanup(m_context.device, m_context.memory); });
            }
        }
    }
}

void Renderer::upload_geometry_data(Core::Geometry *const g)
{
    PROFILING_EVENT()
    Core::RenderData *rd = get_render_data(g);
    if (!rd->loadedOnGPU)
    {
        const Core::GeometricData *gd = g->get_geometric_data();

        size_t vboSize = sizeof(gd->vertexData[0]) * gd->vertexData.size();
        size_t iboSize = sizeof(gd->vertexIndex[0]) * gd->vertexIndex.size();

        m_context.upload_geometry(rd->vbo, vboSize, gd->vertexData.data(), rd->ibo, iboSize, gd->vertexIndex.data(),
                                  g->indexed());

        rd->indexCount = gd->vertexIndex.size();
        rd->vertexCount = gd->vertexIndex.size();
        rd->loadedOnGPU = true;
    }
}

void Renderer::init_resources()
{
    for (size_t i = 0; i < m_context.frames.size(); i++)
    {
        // Global Buffer
        Graphics::Buffer globalBuffer;
        const size_t globalStrideSize =
            (Graphics::utils::pad_uniform_buffer_size(sizeof(Graphics::CameraUniforms), m_context.gpu) +
             Graphics::utils::pad_uniform_buffer_size(sizeof(Graphics::SceneUniforms), m_context.gpu));
        globalBuffer.init(m_context.memory, globalStrideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)globalStrideSize);
        m_context.frames[i].uniformBuffers.push_back(globalBuffer);

        // Object Buffer
        Graphics::Buffer objectBuffer;
        const size_t objectStrideSize =
            (Graphics::utils::pad_uniform_buffer_size(sizeof(Graphics::ObjectUniforms), m_context.gpu) +
             Graphics::utils::pad_uniform_buffer_size(sizeof(Graphics::MaterialUniforms), m_context.gpu));
        objectBuffer.init(m_context.memory, VK_MAX_OBJECTS * objectStrideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)objectStrideSize);
        m_context.frames[i].uniformBuffers.push_back(objectBuffer);
    }

    // Setup dummy texture in case materials dont have textures
    Core::Texture::DEBUG_TEXTURE = new Core::Texture();
    unsigned char texture_data[4] = {0, 0, 0, 0};
    Core::Texture::DEBUG_TEXTURE->set_image_cache(texture_data, {2, 2}, 4);
    Core::Texture::DEBUG_TEXTURE->set_use_mipmaps(false);

    m_context.upload_texture_image(get_image(Core::Texture::DEBUG_TEXTURE), false);
}

void Renderer::clean_Resources()
{
    for (size_t i = 0; i < m_context.frames.size(); i++)
    {
        for (Graphics::Buffer &buffer : m_context.frames[i].uniformBuffers)
        {
            buffer.cleanup(m_context.memory);
        }
    }
    get_image(Core::Texture::DEBUG_TEXTURE)->cleanup(m_context.device, m_context.memory);
}
} // namespace Systems

VULKAN_ENGINE_NAMESPACE_END