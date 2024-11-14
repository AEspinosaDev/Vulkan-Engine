#include <engine/core/renderpasses/shadow_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void ShadowPass::setup_attachments() {

    m_attachments.resize(1);

    m_attachments[0] = Graphics::Attachment(static_cast<VkFormat>(m_depthFormat),
                                            VK_SAMPLE_COUNT_1_BIT,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                            AttachmentType::DEPTH_ATTACHMENT,
                                            VK_IMAGE_ASPECT_DEPTH_BIT,
                                            VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                                            VK_FILTER_LINEAR,
                                            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    // Depdencies

    m_dependencies.resize(2);

    m_dependencies[0] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

    m_dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

    m_dependencies[1] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

    m_dependencies[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_dependencies[1].srcSubpass    = 0;
    m_dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;

    m_isResizeable = false;
}
void ShadowPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    m_device->create_descriptor_pool(
        m_descriptorPool, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding camBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    LayoutBinding sceneBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    LayoutBinding shadowBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    LayoutBinding envBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
    LayoutBinding iblBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT,
                                {camBufferBinding, sceneBufferBinding, shadowBinding, envBinding, iblBinding});

    // PER-OBJECT SET
    LayoutBinding objectBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    LayoutBinding materialBufferBinding(
        UniformDataType::DYNAMIC_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    m_descriptorPool.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, {objectBufferBinding, materialBufferBinding});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[0],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(
            &frames[i].uniformBuffers[0],
            sizeof(SceneUniforms),
            Utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_device->get_GPU()),
            &m_descriptors[i].globalDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[1],
                                              sizeof(ObjectUniforms),
                                              0,
                                              &m_descriptors[i].objectDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(
            &frames[i].uniformBuffers[1],
            sizeof(MaterialUniforms),
            Utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_device->get_GPU()),
            &m_descriptors[i].objectDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1);
    }
}
void ShadowPass::setup_shader_passes() {

    // DEPTH PASSES

    PipelineSettings settings{};
    settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                       {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                       {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    settings.attributes             = {{VertexAttributeType::POSITION, true},
                                       {VertexAttributeType::NORMAL, false},
                                       {VertexAttributeType::UV, false},
                                       {VertexAttributeType::TANGENT, false},
                                       {VertexAttributeType::COLOR, false}};
    settings.dynamicStates          = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_DEPTH_BIAS,
                                       VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
                                       VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                                       VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                                       VK_DYNAMIC_STATE_CULL_MODE};
    settings.blendAttachments       = {};

    ShaderPass* depthPass =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/shadows/shadows_geom.glsl");
    depthPass->settings = settings;
    depthPass->build_shader_stages();
    depthPass->build(m_handle, m_descriptorPool, m_extent);
    m_shaderPasses["shadow"] = depthPass;

    ShaderPass* depthLinePass =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/shadows/shadows_line_geom.glsl");
    depthLinePass->settings             = settings;
    depthLinePass->settings.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    depthLinePass->settings.poligonMode = VK_POLYGON_MODE_LINE;
    depthLinePass->build_shader_stages();
    depthLinePass->build(m_handle, m_descriptorPool, m_extent);
    m_shaderPasses["shadowLine"] = depthLinePass;
}

void ShadowPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer* cmd = currentFrame.commandBuffer;
    cmd->begin_renderpass(m_handle, m_framebuffers[presentImageIndex],m_extent, m_attachments);
    cmd->set_viewport(m_extent);

    cmd->set_depth_bias_enable(true);
    float depthBiasConstant = 0.0;
    float depthBiasSlope    = 0.0f;
    cmd->set_depth_bias(depthBiasConstant, 0.0f, depthBiasSlope);

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
                    IMaterial* mat = m->get_material(i);

                    ShaderPass* shaderPass =
                        mat->get_shaderpass_ID() != "hairstr" && mat->get_shaderpass_ID() != "hairstr2"
                            ? m_shaderPasses["shadow"]
                            : m_shaderPasses["shadowLine"];

                    cmd->set_depth_test_enable(mat->get_parameters().depthTest);
                    cmd->set_depth_write_enable(mat->get_parameters().depthWrite);
                    cmd->set_cull_mode(mat->get_parameters().faceCulling ? mat->get_parameters().culling
                                                                         : CullingMode::NO_CULLING);

                    cmd->bind_shaderpass(*shaderPass);
                    // GLOBAL LAYOUT BINDING
                    cmd->bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
                    // PER OBJECT LAYOUT BINDING
                    cmd->bind_descriptor_set(m_descriptors[currentFrame.index].objectDescritor,
                                             1,
                                             *shaderPass,
                                             {objectOffset, objectOffset});

                    // DRAW
                    Geometry* g = m->get_geometry(i);
                    cmd->draw_geometry(*get_VAO(g));
                }
            }
            mesh_idx++;
        }
    }

    cmd->end_renderpass();
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END