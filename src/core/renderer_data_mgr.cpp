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
#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::update_global_data(Scene *const scene)
{
    /*
    CAMERA UNIFORMS LOAD
    */
    Camera *camera = scene->get_active_camera();
    if (camera->is_dirty())
        camera->set_projection(m_window->get_extent().width, m_window->get_extent().height);
    CameraUniforms camData;
    camData.view = camera->get_view();
    camData.proj = camera->get_projection();
    camData.viewProj = camera->get_projection() * camera->get_view();
    camData.position = Vec4(camera->get_position(), 0.0f);
    camData.screenExtent = {m_window->get_extent().width, m_window->get_extent().height};

    m_context.frames[m_currentFrame].uniformBuffers[GLOBAL_LAYOUT].upload_data(m_context.memory, &camData,
                                                                               sizeof(CameraUniforms), 0);

    /*
    SCENE UNIFORMS LOAD
    */
    SceneUniforms sceneParams;
    sceneParams.fogParams = {camera->get_near(), camera->get_far(), scene->get_fog_intensity(),
                             scene->is_fog_enabled()};
    sceneParams.fogColorAndSSAO = Vec4(scene->get_fog_color(), scene->is_ssao_enabled());
    sceneParams.SSAOtype = 0;
    sceneParams.emphasizeAO = false;
    sceneParams.ambientColor = Vec4(scene->get_ambient_color(), scene->get_ambient_intensity());

    std::vector<Light *> lights = scene->get_lights();
    if (lights.size() > VK_MAX_LIGHTS)
        std::sort(lights.begin(), lights.end(), [=](Light *a, Light *b) {
            return math::length(a->get_position() - camera->get_position()) <
                   math::length(b->get_position() - camera->get_position());
        });

    size_t lightIdx{0};
    for (Light *l : lights)
    {
        if (l->is_active())
        {
            sceneParams.lightUniforms[lightIdx] = l->get_uniforms(camera->get_view());
            Mat4 depthProjectionMatrix =
                math::perspective(math::radians(l->m_shadow.fov), 1.0f, l->m_shadow.nearPlane, l->m_shadow.farPlane);
            Mat4 depthViewMatrix = math::lookAt(l->m_transform.position, l->m_shadow.target, Vec3(0, 1, 0));
            sceneParams.lightUniforms[lightIdx].viewProj = depthProjectionMatrix * depthViewMatrix;
            lightIdx++;
        }
        if (lightIdx >= VK_MAX_LIGHTS)
            break;
    }
    sceneParams.numLights = static_cast<int>(lights.size());

    m_context.frames[m_currentFrame].uniformBuffers[GLOBAL_LAYOUT].upload_data(
        m_context.memory, &sceneParams, sizeof(SceneUniforms),
        utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_context.gpu));
}
void Renderer::update_object_data(Scene *const scene)
{

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {
        std::vector<Mesh *> meshes;
        std::vector<Mesh *> blendMeshes;

        for (Mesh *m : scene->get_meshes())
        {
            if (m->get_material())
                m->get_material()->get_parameters().blending ? blendMeshes.push_back(m) : meshes.push_back(m);
        }

        // Calculate distance
        if (!blendMeshes.empty())
        {

            std::map<float, Mesh *> sorted;
            for (unsigned int i = 0; i < blendMeshes.size(); i++)
            {
                float distance =
                    glm::distance(scene->get_active_camera()->get_position(), blendMeshes[i]->get_position());
                sorted[distance] = blendMeshes[i];
            }

            // SECOND = TRANSPARENT OBJECTS SORTED FROM NEAR TO FAR
            for (std::map<float, Mesh *>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
            {
                meshes.push_back(it->second);
            }
            scene->set_meshes(meshes);
        }

        unsigned int mesh_idx = 0;
        for (Mesh *m : scene->get_meshes())
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

                    ObjectUniforms objectData;
                    objectData.model = m->get_model_matrix();
                    objectData.otherParams1 = {m->is_affected_by_fog(), m->get_recive_shadows(),
                    m->get_cast_shadows(), false}; objectData.otherParams2 = {m->is_selected(), 0.0, 0.0, 0.0};
                    m_context.frames[m_currentFrame].uniformBuffers[1].upload_data(m_context.memory, &objectData,
                    sizeof(ObjectUniforms), objectOffset);

                    // void *meshData = nullptr;
                    // size_t size = 0;
                    // m->get_uniform_data(meshData, size);
                    // m_context.frames[m_currentFrame].uniformBuffers[OBJECT_LAYOUT].upload_data(
                    //     m_context.memory, meshData, size, objectOffset);
                    // free(meshData);

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        // Object vertex buffer setup
                        Geometry *g = m->get_geometry(i);
                        upload_geometry_data(g);

                        // Object material setup
                        Material *mat = m->get_material(g->get_material_ID());
                        if (mat)
                            upload_material_textures(mat);
                        else
                            upload_material_textures(Material::DEBUG_MATERIAL);

                        // ObjectUniforms materialData;
                        MaterialUniforms materialData = mat->get_uniforms();
                        m_context.frames[m_currentFrame].uniformBuffers[OBJECT_LAYOUT].upload_data(
                            m_context.memory, &materialData, sizeof(MaterialUniforms),
                            objectOffset + utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_context.gpu));
                    }
                }
            }
            mesh_idx++;
        }
    }
}

void Renderer::upload_material_textures(Material *const mat)
{
    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        Texture *texture = pair.second;
        if (texture && texture->data_loaded())
        {
            if (!texture->is_buffer_loaded())
            {
                m_context.upload_texture_image(texture->m_image, static_cast<const void *>(texture->m_tmpCache),
                                               (VkFormat)texture->m_settings.format,
                                               (VkFilter)texture->m_settings.filter,
                                               (VkSamplerAddressMode)texture->m_settings.adressMode,
                                               texture->m_settings.anisotropicFilter, texture->m_settings.useMipmaps);
                texture->m_buffer_loaded = true;
                m_deletionQueue.push_function([=]() { texture->m_image.cleanup(m_context.device, m_context.memory); });
            }
        }
    }
}

void Renderer::upload_geometry_data(Geometry *const g)
{
    if (!g->get_render_data().loaded)
    {
        size_t vboSize = sizeof(g->get_vertex_data()[0]) * g->get_vertex_data().size();
        size_t iboSize = sizeof(g->get_vertex_index()[0]) * g->get_vertex_index().size();
        RenderData rd = g->get_render_data();
        m_context.upload_geometry(rd.vbo, vboSize, g->get_vertex_data().data(), rd.ibo, iboSize,
                                  g->get_vertex_index().data(), g->indexed());
        rd.loaded = true;
        g->set_render_data(rd);
    }
}

void Renderer::init_resources()
{
    for (size_t i = 0; i < m_context.frames.size(); i++)
    {
        // Global Buffer
        Buffer globalBuffer;
        const size_t globalStrideSize = (utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_context.gpu) +
                                         utils::pad_uniform_buffer_size(sizeof(SceneUniforms), m_context.gpu));
        globalBuffer.init(m_context.memory, globalStrideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)globalStrideSize);
        m_context.frames[i].uniformBuffers.push_back(globalBuffer);

        // Object Buffer
        Buffer objectBuffer;
        const size_t objectStrideSize = (utils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_context.gpu) +
                                         utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_context.gpu));
        objectBuffer.init(m_context.memory, VK_MAX_OBJECTS * objectStrideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)objectStrideSize);
        m_context.frames[i].uniformBuffers.push_back(objectBuffer);
    }

    // Setup dummy texture in case materials dont have textures
    Texture::DEBUG_TEXTURE = new Texture();
    Texture::DEBUG_TEXTURE->load_image(ENGINE_RESOURCES_PATH "textures/dummy.jpg", false);
    Texture::DEBUG_TEXTURE->set_use_mipmaps(false);

    m_context.upload_texture_image(
        Texture::DEBUG_TEXTURE->m_image, static_cast<const void *>(Texture::DEBUG_TEXTURE->m_tmpCache),
        (VkFormat)Texture::DEBUG_TEXTURE->m_settings.format, (VkFilter)Texture::DEBUG_TEXTURE->m_settings.filter,
        (VkSamplerAddressMode)Texture::DEBUG_TEXTURE->m_settings.adressMode,
        Texture::DEBUG_TEXTURE->m_settings.anisotropicFilter, false);
    Texture::DEBUG_TEXTURE->m_buffer_loaded = true;
}

void Renderer::clean_Resources()
{
    for (size_t i = 0; i < m_context.frames.size(); i++)
    {
        for (Buffer &buffer : m_context.frames[i].uniformBuffers)
        {
            buffer.cleanup(m_context.memory);
        }
    }
    Texture::DEBUG_TEXTURE->m_image.cleanup(m_context.device, m_context.memory);
}

VULKAN_ENGINE_NAMESPACE_END