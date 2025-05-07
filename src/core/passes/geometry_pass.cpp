#include <engine/core/passes/geometry_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {
void GeometryPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) {

    //////////////////////
    // G - BUFFER
    /////////////////////
    attachments.resize(5);

    // Normals + Depth
    attachments[0] = Graphics::AttachmentConfig(m_floatFormat,
                                                1,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST);
    // Albedo + Opacity
    attachments[1] = Graphics::AttachmentConfig(RGBA_8U,
                                                1,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST);
    // Material + ID
    attachments[2] = Graphics::AttachmentConfig(RGBA_8U,
                                                1,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST);
    // Velocity + Emissive strength
    attachments[3] = Graphics::AttachmentConfig(m_floatFormat,
                                                1,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST);

    // Depth
    attachments[4]                                           = Graphics::AttachmentConfig(m_depthFormat,
                                                1,
                                                LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                                LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                                DEPTH_ATTACHMENT,
                                                ASPECT_DEPTH);
    attachments[4].imageConfig.clearValue.depthStencil.depth = 0.0f; // Inverse Z

    // Depdencies
    dependencies.resize(2);

    dependencies[0]               = Graphics::SubPassDependency(STAGE_BOTTOM_OF_PIPE, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[0].srcAccessMask = ACCESS_MEMORY_READ;
    dependencies[1]               = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_BOTTOM_OF_PIPE, ACCESS_MEMORY_READ);
    dependencies[1].srcAccessMask = ACCESS_COLOR_ATTACHMENT_WRITE;
    dependencies[1].srcSubpass    = 0;
    dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
}
void GeometryPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    m_descriptorPool = m_device->create_descriptor_pool(ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding camBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding sceneBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding shadowBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding envBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding iblBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding accelBinding(UNIFORM_ACCELERATION_STRUCTURE, SHADER_STAGE_FRAGMENT, 5);
    LayoutBinding noiseBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 6);
    LayoutBinding skyBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 7);
    m_descriptorPool.set_layout(
        GLOBAL_LAYOUT, {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding, accelBinding, noiseBinding, skyBinding});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding materialBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    // MATERIAL TEXTURE SET
    LayoutBinding materialTextureBufferBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0, MAX_TEXTURES);
    m_descriptorPool.set_layout(OBJECT_TEXTURE_LAYOUT,
                                {materialTextureBufferBinding},
                                0,
                                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptors[i].globalDescritor.update(&frames[i].uniformBuffers[GLOBAL_LAYOUT], sizeof(CameraUniforms), 0, UNIFORM_DYNAMIC_BUFFER, 0);
        m_descriptors[i].globalDescritor.update(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                                sizeof(SceneUniforms),
                                                m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),

                                                UNIFORM_DYNAMIC_BUFFER,
                                                1);

        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::FALLBACK_TEXTURE), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 3);
        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::textureResources[0]), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);
        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::FALLBACK_TEXTURE), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 7);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptors[i].objectDescritor.update(&frames[i].uniformBuffers[OBJECT_LAYOUT], sizeof(ObjectUniforms), 0, UNIFORM_DYNAMIC_BUFFER, 0);
        m_descriptors[i].objectDescritor.update(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                                sizeof(MaterialUniforms),
                                                m_device->pad_uniform_buffer_size(sizeof(MaterialUniforms)),
                                                UNIFORM_DYNAMIC_BUFFER,
                                                1);
        // Set up enviroment fallback texture
        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::FALLBACK_CUBEMAP), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 3);
        m_descriptors[i].globalDescritor.update(get_image(ResourceManager::FALLBACK_CUBEMAP), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 4);

        // Textures
        m_descriptorPool.allocate_variable_descriptor_set(OBJECT_TEXTURE_LAYOUT, &m_descriptors[i].textureDescritor, MAX_TEXTURES);
    }
}
void GeometryPass::setup_shader_passes() {

    // Geometry
    GraphicShaderPass* geomPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/deferred/geometry.glsl"));
    geomPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, true}};
    geomPass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, true}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, true}, {COLOR_ATTRIBUTE, false}};
    geomPass->graphicSettings.dynamicStates    = {VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR,
                                                  VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                                                  VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                                                  VK_DYNAMIC_STATE_CULL_MODE};
    geomPass->graphicSettings.blendAttachments = {Init::color_blend_attachment_state(false),
                                                  Init::color_blend_attachment_state(false),
                                                  Init::color_blend_attachment_state(false),
                                                  Init::color_blend_attachment_state(false)};
    geomPass->graphicSettings.depthOp          = VK_COMPARE_OP_GREATER_OR_EQUAL;

    geomPass->build_shader_stages();
    geomPass->build(m_descriptorPool);

    m_shaderPasses["geometryTri"] = geomPass;

    GraphicShaderPass* geomLinePass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/deferred/line_to_tri_geometry.glsl"));
    geomLinePass->settings.descriptorSetLayoutIDs = geomPass->settings.descriptorSetLayoutIDs;
    geomLinePass->graphicSettings                 = geomPass->graphicSettings;
    geomLinePass->graphicSettings.topology        = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    geomLinePass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, true}, {COLOR_ATTRIBUTE, false}};

    geomLinePass->build_shader_stages();
    geomLinePass->build(m_descriptorPool);

    m_shaderPasses["geometryLineTri"] = geomLinePass;

    GraphicShaderPass* linePass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/deferred/line_geometry.glsl"));
    linePass->settings.descriptorSetLayoutIDs = geomPass->settings.descriptorSetLayoutIDs;
    linePass->graphicSettings                 = geomPass->graphicSettings;
    linePass->graphicSettings.topology        = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    linePass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, true}, {COLOR_ATTRIBUTE, false}};

    linePass->build_shader_stages();
    linePass->build(m_descriptorPool);

    m_shaderPasses["geometryLine"] = linePass;

    GraphicShaderPass* skyboxPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/deferred/skybox.glsl"));
    skyboxPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, false}, {OBJECT_TEXTURE_LAYOUT, false}};
    skyboxPass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, false}, {TANGENT_ATTRIBUTE, false}, {COLOR_ATTRIBUTE, false}};
    skyboxPass->graphicSettings.dynamicStates    = geomPass->graphicSettings.dynamicStates;
    skyboxPass->graphicSettings.blendAttachments = geomPass->graphicSettings.blendAttachments;

    skyboxPass->build_shader_stages();
    skyboxPass->build(m_descriptorPool);

    m_shaderPasses["skybox"] = skyboxPass;
}
void GeometryPass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {
        // Skybox
        if (scene->get_skybox())
        {
            if (scene->get_skybox()->is_active())
            {

                cmd.set_depth_test_enable(false);
                cmd.set_depth_write_enable(false);
                cmd.set_cull_mode(CullingMode::NO_CULLING);

                ShaderPass* shaderPass = m_shaderPasses["skybox"];

                // Bind pipeline
                cmd.bind_shaderpass(*shaderPass);

                // GLOBAL LAYOUT BINDING
                cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});

                cmd.draw_geometry(*get_VAO(scene->get_skybox()->get_box()));
            }
        }

        ShaderPass* shaderPass;
        shaderPass = m_shaderPasses["geometryTri"];
        cmd.bind_shaderpass(*shaderPass);
        // GLOBAL LAYOUT BINDING
        cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
        // TEXTURE LAYOUT BINDING
        if (shaderPass->settings.descriptorSetLayoutIDs[OBJECT_TEXTURE_LAYOUT])
            cmd.bind_descriptor_set(m_descriptors[currentFrame.index].textureDescritor, 2, *shaderPass);

        Topology prevTopology = Topology::TRIANGLES;

        unsigned int mesh_idx = 0;
        for (Mesh* m : scene->get_meshes())
        {
            if (m)
            {
                if (m->is_active() &&    // Check if is active
                    m->get_geometry() && // Check if has geometry
                    (scene->get_active_camera()->get_frustrum_culling() && m->get_bounding_volume()
                         ? m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum())
                         : true)) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset = currentFrame.uniformBuffers[1].strideSize * mesh_idx;

                    Geometry*  g   = m->get_geometry();
                    IMaterial* mat = m->get_material();

                    cmd.set_depth_test_enable(mat->get_parameters().depthTest);
                    cmd.set_depth_write_enable(mat->get_parameters().depthWrite);
                    cmd.set_cull_mode(mat->get_parameters().faceCulling ? mat->get_parameters().culling : CullingMode::NO_CULLING);

                    if (prevTopology != g->get_properties().topology)
                    {
                        // Choose shader pass based on topology
                        switch (g->get_properties().topology)
                        {
                        case Topology::TRIANGLES:
                            shaderPass = m_shaderPasses["geometryTri"];
                            break;
                        case Topology::LINES_TO_TRIANGLES:
                            shaderPass = m_shaderPasses["geometryLineTri"];
                            break;
                        case Topology::LINES:
                            shaderPass = m_shaderPasses["geometryLine"];
                            break;
                        default:
                            break;
                        }
                        // Rebind global descriptors
                        cmd.bind_shaderpass(*shaderPass);
                        // GLOBAL LAYOUT BINDING
                        cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
                        // TEXTURE LAYOUT BINDING
                        if (shaderPass->settings.descriptorSetLayoutIDs[OBJECT_TEXTURE_LAYOUT])
                            cmd.bind_descriptor_set(m_descriptors[currentFrame.index].textureDescritor, 2, *shaderPass);
                    }
                    // PER OBJECT LAYOUT BINDING
                    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].objectDescritor, 1, *shaderPass, {objectOffset, objectOffset});

                    // DRAW
                    cmd.draw_geometry(*get_VAO(g));
                }
            }
            mesh_idx++;
        }
    }

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);
}

void GeometryPass::link_input_attachments() {

    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        m_descriptors[i].globalDescritor.update(m_inAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 3);
        m_descriptors[i].globalDescritor.update(m_inAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 4);
        m_descriptors[i].globalDescritor.update(m_inAttachments[2], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 7);
    }
}

void GeometryPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    uint32_t meshIdx = 0;
    for (Mesh* m : scene->get_meshes())
    {
        if (m)
        {
            auto g   = m->get_geometry();
            auto mat = m->get_material();
            setup_material_descriptor(mat, meshIdx);
        }
        meshIdx++;
    }
}

void GeometryPass::setup_material_descriptor(IMaterial* mat, uint32_t meshIdx) {
    auto textures = mat->get_textures();
    for (auto pair : textures)
    {
        ITexture* texture = pair.second;
        if (texture && texture->loaded_on_GPU())
        {

            for (size_t i = 0; i < m_descriptors.size(); i++)
            {
                uint32_t bindingPoint = 0;
                m_descriptors[i].textureDescritor.update(
                    get_image(texture), LAYOUT_SHADER_READ_ONLY_OPTIMAL, bindingPoint, UNIFORM_COMBINED_IMAGE_SAMPLER, pair.first + 6 * meshIdx);
            }
        }
    }
}
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
