#include <engine/core/passes/shadow_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void ShadowPass::setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                   std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::AttachmentInfo(m_depthFormat,
                                              1,
                                              LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                              LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                              IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
                                              DEPTH_ATTACHMENT,
                                              ASPECT_DEPTH,
                                              TEXTURE_2D_ARRAY,
                                              FILTER_LINEAR,
                                              ADDRESS_MODE_CLAMP_TO_BORDER);

    // Depdencies

    dependencies.resize(2);

    dependencies[0] = Graphics::SubPassDependency(
        STAGE_FRAGMENT_SHADER, STAGE_EARLY_FRAGMENT_TESTS, ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE);

    dependencies[0].srcAccessMask = ACCESS_SHADER_READ;

    dependencies[1] = Graphics::SubPassDependency(
        STAGE_LATE_FRAGMENT_TESTS, STAGE_FRAGMENT_SHADER, ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE);

    dependencies[1].srcAccessMask = ACCESS_SHADER_READ;
    dependencies[1].srcSubpass    = 0;
    dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;

    m_isResizeable = false;
}
void ShadowPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

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
    m_descriptorPool.set_layout(
        GLOBAL_LAYOUT, {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding materialBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[0],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[0],
                                              sizeof(SceneUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[1],
                                              sizeof(ObjectUniforms),
                                              0,
                                              &m_descriptors[i].objectDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[1],
                                              sizeof(MaterialUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(MaterialUniforms)),
                                              &m_descriptors[i].objectDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);
    }
}
void ShadowPass::setup_shader_passes() {

    // DEPTH PASSES

    PipelineSettings        settings{};
    GraphicPipelineSettings gfxSettings{};
    settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, false}};
    gfxSettings.attributes          = {{POSITION_ATTRIBUTE, true},
                                       {NORMAL_ATTRIBUTE, false},
                                       {UV_ATTRIBUTE, false},
                                       {TANGENT_ATTRIBUTE, false},
                                       {COLOR_ATTRIBUTE, false}};
    gfxSettings.dynamicStates       = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_DEPTH_BIAS,
                                       VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
                                       VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                                       VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                                       VK_DYNAMIC_STATE_CULL_MODE};
    gfxSettings.blendAttachments    = {};

    GraphicShaderPass* depthPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/shadows/shadows_geom.glsl");
    depthPass->settings        = settings;
    depthPass->graphicSettings = gfxSettings;
    depthPass->build_shader_stages();
    depthPass->build(m_descriptorPool);
    m_shaderPasses["shadow"] = depthPass;

    GraphicShaderPass* depthLinePass =
        new GraphicShaderPass(m_device->get_handle(),
                              m_renderpass,
                              m_imageExtent,
                              ENGINE_RESOURCES_PATH "shaders/shadows/shadows_line_geom.glsl");
    depthLinePass->settings                    = settings;
    depthLinePass->graphicSettings             = gfxSettings;
    depthLinePass->graphicSettings.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    depthLinePass->graphicSettings.poligonMode = VK_POLYGON_MODE_LINE;
    depthLinePass->build_shader_stages();
    depthLinePass->build(m_descriptorPool);
    m_shaderPasses["shadowLine"] = depthLinePass;
}

void ShadowPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[presentImageIndex]);
    cmd.set_viewport(m_imageExtent);

    cmd.set_depth_bias_enable(true);
    float depthBiasConstant = 0.0;
    float depthBiasSlope    = 0.0f;
    cmd.set_depth_bias(depthBiasConstant, 0.0f, depthBiasSlope);

    int mesh_idx = 0;
    for (Mesh* m : scene->get_meshes())
    {
        if (m)
        {
            if (m->is_active() && m->cast_shadows() && m->get_num_geometries() > 0)
            {
                uint32_t objectOffset = currentFrame.uniformBuffers[1].strideSize * mesh_idx;

                for (size_t i = 0; i < m->get_num_geometries(); i++)
                {

                    // Setup per object render state
                    Geometry*  g   = m->get_geometry(i);
                    IMaterial* mat = m->get_material(g->get_material_ID());

                    ShaderPass* shaderPass =
                        mat->get_shaderpass_ID() != "hairstr" && mat->get_shaderpass_ID() != "hairstr2"
                            ? m_shaderPasses["shadow"]
                            : m_shaderPasses["shadowLine"];

                    cmd.set_depth_test_enable(mat->get_parameters().depthTest);
                    cmd.set_depth_write_enable(mat->get_parameters().depthWrite);
                    cmd.set_cull_mode(mat->get_parameters().faceCulling ? mat->get_parameters().culling
                                                                        : CullingMode::NO_CULLING);

                    cmd.bind_shaderpass(*shaderPass);
                    // GLOBAL LAYOUT BINDING
                    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
                    // PER OBJECT LAYOUT BINDING
                    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].objectDescritor,
                                            1,
                                            *shaderPass,
                                            {objectOffset, objectOffset});

                    // DRAW
                    cmd.draw_geometry(*get_VAO(g));
                }
            }
            mesh_idx++;
        }
    }

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END