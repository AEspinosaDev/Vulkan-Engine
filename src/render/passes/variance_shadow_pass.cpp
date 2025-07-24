#include <engine/render/passes/variance_shadow_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Render {

void VarianceShadowPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(2);

    attachments[0] = Graphics::AttachmentConfig(m_format,
                                                1,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                                COLOR_ATTACHMENT,
                                                ASPECT_COLOR,
                                                TEXTURE_2D_ARRAY,
                                                FILTER_LINEAR,
                                                ADDRESS_MODE_CLAMP_TO_BORDER);

    attachments[1] = Graphics::AttachmentConfig(m_depthFormat,
                                                1,
                                                LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
                                                DEPTH_ATTACHMENT,
                                                ASPECT_DEPTH,
                                                TEXTURE_2D_ARRAY);

    // Depdencies
    dependencies.resize(2);

    dependencies[0] = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[1] = Graphics::SubPassDependency(STAGE_EARLY_FRAGMENT_TESTS, STAGE_EARLY_FRAGMENT_TESTS, ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE);

    m_isResizeable = false;
}
void VarianceShadowPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    m_descriptorPool = m_device->create_descriptor_pool(ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding camBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding sceneBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding shadowBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding envBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding iblBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding materialBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptors[i].globalDescritor.update(&frames[i].uniformBuffers[GLOBAL_LAYOUT], sizeof( Core::Camera::GPUPayload), 0, UNIFORM_DYNAMIC_BUFFER, 0);
        m_descriptors[i].globalDescritor.update(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                                sizeof( Core::Scene::GPUPayload),
                                                m_device->pad_uniform_buffer_size(sizeof( Core::Camera::GPUPayload)),

                                                UNIFORM_DYNAMIC_BUFFER,
                                                1);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptors[i].objectDescritor.update(&frames[i].uniformBuffers[OBJECT_LAYOUT], sizeof(Core::Object3D::GPUPayload), 0, UNIFORM_DYNAMIC_BUFFER, 0);
        m_descriptors[i].objectDescritor.update(&frames[i].uniformBuffers[OBJECT_LAYOUT],
                                                sizeof( Core::IMaterial::GPUPayload),
                                                m_device->pad_uniform_buffer_size(sizeof( Core::IMaterial::GPUPayload)),
                                                UNIFORM_DYNAMIC_BUFFER,
                                                1);
    }
}
void VarianceShadowPass::setup_shader_passes() {

    // DEPTH PASSES

    PipelineSettings        settings{};
    GraphicPipelineSettings gfxSettings{};
    settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}, {OBJECT_LAYOUT, true}, {OBJECT_TEXTURE_LAYOUT, false}};
    gfxSettings.attributes          = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, false}, {TANGENT_ATTRIBUTE, false}, {COLOR_ATTRIBUTE, false}};
    gfxSettings.dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                 VK_DYNAMIC_STATE_SCISSOR,
                                 VK_DYNAMIC_STATE_DEPTH_BIAS,
                                 VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
                                 VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                                 VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                                 VK_DYNAMIC_STATE_CULL_MODE};
    // settings.blendAttachments       = {};

    GraphicShaderPass* depthPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/shadows/vsm_geom.glsl"));
    depthPass->settings        = settings;
    depthPass->graphicSettings = gfxSettings;
    depthPass->build_shader_stages();
    depthPass->build(m_descriptorPool);
    m_shaderPasses["shadowTri"] = depthPass;

    GraphicShaderPass* depthLinePass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/shadows/vsm_line_geom.glsl"));
    depthLinePass->settings                    = settings;
    depthLinePass->graphicSettings             = gfxSettings;
    depthLinePass->graphicSettings.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    depthLinePass->graphicSettings.poligonMode = VK_POLYGON_MODE_LINE;
    depthLinePass->build_shader_stages();
    depthLinePass->build(m_descriptorPool);
    m_shaderPasses["shadowLine"] = depthLinePass;
}

void VarianceShadowPass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()
    CommandBuffer cmd = currentFrame.commandBuffer;
    if ((Core::Light::get_non_raytraced_count() == 0) || scene->get_lights().empty())
    {
        if (m_outAttachments[0]->currentLayout == LAYOUT_UNDEFINED)
            cmd.pipeline_barrier(*m_outAttachments[0],
                                 LAYOUT_UNDEFINED,
                                 LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 ACCESS_NONE,
                                 ACCESS_SHADER_READ,
                                 STAGE_TOP_OF_PIPE,
                                 STAGE_FRAGMENT_SHADER);

        return;
    }

    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
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
            if (m->is_active() && m->cast_shadows() && m->get_geometry())
            {
                uint32_t objectOffset = currentFrame.uniformBuffers[1].strideSize * mesh_idx;

                // Setup per object render state
                auto g   = m->get_geometry();
                auto mat = m->get_material();

                ShaderPass* shaderPass;
                switch (g->get_properties().topology)
                {
                case Topology::TRIANGLES:
                    shaderPass = m_shaderPasses["shadowTri"];
                    break;
                case Topology::LINES_TO_TRIANGLES:
                    shaderPass = m_shaderPasses["shadowLine"];
                    break;
                case Topology::LINES:
                    shaderPass = m_shaderPasses["shadowLine"];
                    break;
                default:
                    break;
                }

                cmd.set_depth_test_enable(mat->get_parameters().depthTest);
                cmd.set_depth_write_enable(mat->get_parameters().depthWrite);
                cmd.set_cull_mode(mat->get_parameters().faceCulling ? mat->get_parameters().culling : CullingMode::NO_CULLING);

                cmd.bind_shaderpass(*shaderPass);
                // GLOBAL LAYOUT BINDING
                cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
                // PER OBJECT LAYOUT BINDING
                cmd.bind_descriptor_set(m_descriptors[currentFrame.index].objectDescritor, 1, *shaderPass, {objectOffset, objectOffset});

                // DRAW
                cmd.draw_geometry(*get_VAO(g));
            }
            mesh_idx++;
        }
    }

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END