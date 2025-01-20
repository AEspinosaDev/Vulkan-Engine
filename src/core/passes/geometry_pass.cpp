#include <engine/core/passes/geometry_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {
void GeometryPass::setup_attachments(std::vector<Graphics::AttachmentInfo>&        attachments,
                                     std::vector<Graphics::SubPassDependency>& dependencies) {

    //////////////////////
    // G - BUFFER
    /////////////////////
    attachments.resize(6);

    // Positions
    attachments[0] = Graphics::AttachmentInfo(SRGBA_32F,
                                          1,
                                          LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED);
    // Normals
    attachments[1] = Graphics::AttachmentInfo(SRGBA_32F,
                                          1,
                                          LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED);
    // Albedo
    attachments[2] = Graphics::AttachmentInfo(SRGBA_32F,
                                          1,
                                          LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED);
    // Material
    attachments[3] = Graphics::AttachmentInfo(RGBA_8U,
                                          1,
                                          LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED);
    // Emissive
    attachments[4] = Graphics::AttachmentInfo(SRGBA_32F,
                                          1,
                                          LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED);

    // Temporal
    // attachments[5] = Graphics::Attachment(m_colorFormat,
    //                                       1,
    //                                       LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                                       LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //                                       IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED);

    // Depth
    attachments[5] = Graphics::AttachmentInfo(m_depthFormat,
                                          1,
                                          LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                          LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
                                          DEPTH_ATTACHMENT,
                                          ASPECT_DEPTH);

    // Depdencies
    dependencies.resize(2);

    dependencies[0] =
        Graphics::SubPassDependency(STAGE_BOTTOM_OF_PIPE, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[0].srcAccessMask = ACCESS_MEMORY_READ;
    dependencies[1] =
        Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_BOTTOM_OF_PIPE, ACCESS_MEMORY_READ);
    dependencies[1].srcAccessMask = ACCESS_COLOR_ATTACHMENT_WRITE;
    dependencies[1].srcSubpass    = 0;
    dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
}
void GeometryPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    m_descriptorPool = m_device->create_descriptor_pool(
        ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding camBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding sceneBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding shadowBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding envBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding iblBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding accelBinding(UNIFORM_ACCELERATION_STRUCTURE, SHADER_STAGE_FRAGMENT, 5);
    LayoutBinding noiseBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 6);
    m_descriptorPool.set_layout(
        GLOBAL_LAYOUT,
        {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding, accelBinding, noiseBinding});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding materialBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    // MATERIAL TEXTURE SET
    LayoutBinding textureBinding1(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding textureBinding2(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding textureBinding3(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding textureBinding4(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding textureBinding5(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding textureBinding6(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5);
    m_descriptorPool.set_layout(
        OBJECT_TEXTURE_LAYOUT,
        {textureBinding1, textureBinding2, textureBinding3, textureBinding4, textureBinding5, textureBinding6});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(SceneUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);

        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_TEXTURE),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);

        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::textureResources[0]),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              6);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                              sizeof(ObjectUniforms),
                                              0,
                                              &m_descriptors[i].objectDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                              sizeof(MaterialUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(MaterialUniforms)),
                                              &m_descriptors[i].objectDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);
        // Set up enviroment fallback texture
        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);
        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              4);
    }
}
void GeometryPass::setup_shader_passes() {

    // Geometry
    GraphicShaderPass* geomPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/deferred/geometry.glsl");
    geomPass->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    geomPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                  {NORMAL_ATTRIBUTE, true},
                                                  {UV_ATTRIBUTE, true},
                                                  {TANGENT_ATTRIBUTE, true},
                                                  {COLOR_ATTRIBUTE, false}};
    geomPass->graphicSettings.dynamicStates    = {VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR,
                                                  VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                                                  VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                                                  VK_DYNAMIC_STATE_CULL_MODE};
    geomPass->graphicSettings.blendAttachments = {Init::color_blend_attachment_state(false),
                                                  Init::color_blend_attachment_state(false),
                                                  Init::color_blend_attachment_state(false),
                                                  Init::color_blend_attachment_state(false),
                                                  Init::color_blend_attachment_state(false)};

    geomPass->build_shader_stages();
    geomPass->build(m_descriptorPool);

    m_shaderPasses["geometry"] = geomPass;

    GraphicShaderPass* skyboxPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent,  ENGINE_RESOURCES_PATH "shaders/deferred/skybox.glsl");
    skyboxPass->settings.descriptorSetLayoutIDs = {
        {GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, false}, {OBJECT_TEXTURE_LAYOUT, false}};
    skyboxPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                    {NORMAL_ATTRIBUTE, false},
                                                    {UV_ATTRIBUTE, false},
                                                    {TANGENT_ATTRIBUTE, false},
                                                    {COLOR_ATTRIBUTE, false}};
    skyboxPass->graphicSettings.dynamicStates    = geomPass->graphicSettings.dynamicStates;
    skyboxPass->graphicSettings.blendAttachments = geomPass->graphicSettings.blendAttachments;
    skyboxPass->graphicSettings.depthOp          = VK_COMPARE_OP_LESS_OR_EQUAL;

    skyboxPass->build_shader_stages();
    skyboxPass->build(m_descriptorPool);

    m_shaderPasses["skybox"] = skyboxPass;
}
void GeometryPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {

        ShaderPass* shaderPass = m_shaderPasses["geometry"];

        unsigned int mesh_idx = 0;
        for (Mesh* m : scene->get_meshes())
        {
            if (m)
            {
                if (m->is_active() &&              // Check if is active
                    m->get_num_geometries() > 0 && // Check if has geometry
                    (scene->get_active_camera()->get_frustrum_culling() && m->get_bounding_volume()
                         ? m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum())
                         : true)) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset = currentFrame.uniformBuffers[1].strideSize * mesh_idx;

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        Geometry*  g   = m->get_geometry(i);
                        IMaterial* mat = m->get_material(g->get_material_ID());

                        cmd.set_depth_test_enable(mat->get_parameters().depthTest);
                        cmd.set_depth_write_enable(mat->get_parameters().depthWrite);
                        cmd.set_cull_mode(mat->get_parameters().faceCulling ? mat->get_parameters().culling
                                                                            : CullingMode::NO_CULLING);

                        // Bind pipeline
                        cmd.bind_shaderpass(*shaderPass);
                        // GLOBAL LAYOUT BINDING
                        cmd.bind_descriptor_set(
                            m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
                        // PER OBJECT LAYOUT BINDING
                        cmd.bind_descriptor_set(m_descriptors[currentFrame.index].objectDescritor,
                                                1,
                                                *shaderPass,
                                                {objectOffset, objectOffset});
                        // TEXTURE LAYOUT BINDING
                        if (shaderPass->settings.descriptorSetLayoutIDs[OBJECT_TEXTURE_LAYOUT])
                            cmd.bind_descriptor_set(mat->get_texture_descriptor(), 2, *shaderPass);

                        // DRAW
                        cmd.draw_geometry(*get_VAO(g));
                    }
                }
            }
            mesh_idx++;
        }
        // Skybox
        if (scene->get_skybox())
        {
            if (scene->get_skybox()->is_active())
            {

                cmd.set_depth_test_enable(true);
                cmd.set_depth_write_enable(true);
                cmd.set_cull_mode(CullingMode::NO_CULLING);

                ShaderPass* shaderPass = m_shaderPasses["skybox"];

                // Bind pipeline
                cmd.bind_shaderpass(*shaderPass);

                // GLOBAL LAYOUT BINDING
                cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});

                cmd.draw_geometry(*get_VAO(scene->get_skybox()->get_box()));
            }
        }
    }

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);
}

void GeometryPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    for (Mesh* m : scene->get_meshes())
    {
        if (m)
        {
            for (size_t i = 0; i < m->get_num_geometries(); i++)
            {
                Geometry*  g   = m->get_geometry(i);
                IMaterial* mat = m->get_material(g->get_material_ID());
                setup_material_descriptor(mat);
            }
        }
    }
}
void GeometryPass::set_envmap_descriptor(Graphics::Image env, Graphics::Image irr) {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        m_descriptorPool.set_descriptor_write(
            &env, LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3);
        m_descriptorPool.set_descriptor_write(
            &irr, LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 4);
    }
}
void GeometryPass::setup_material_descriptor(IMaterial* mat) {
    if (!mat->get_texture_descriptor().allocated)
        m_descriptorPool.allocate_descriptor_set(OBJECT_TEXTURE_LAYOUT, &mat->get_texture_descriptor());

    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        ITexture* texture = pair.second;
        if (texture && texture->loaded_on_GPU())
        {

            // Set texture write
            if (!mat->get_texture_binding_state()[pair.first] || texture->is_dirty())
            {
                m_descriptorPool.set_descriptor_write(
                    get_image(texture), LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->get_texture_descriptor(), pair.first);
                mat->set_texture_binding_state(pair.first, true);
                texture->set_dirty(false);
            }
        } else
        {
            // SET DUMMY TEXTURE
            if (!mat->get_texture_binding_state()[pair.first])
                m_descriptorPool.set_descriptor_write(get_image(ResourceManager::FALLBACK_TEXTURE),
                                                      LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                      &mat->get_texture_descriptor(),
                                                      pair.first);
            mat->set_texture_binding_state(pair.first, true);
        }
    }
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
