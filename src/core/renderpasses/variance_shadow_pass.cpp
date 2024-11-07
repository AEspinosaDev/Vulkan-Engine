#include <engine/core/renderpasses/variance_shadow_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void VarianceShadowPass::setup_attachments() {

    m_attachments.resize(2);

    m_attachments[0] = Graphics::Attachment(static_cast<VkFormat>(m_format),
                                            VK_SAMPLE_COUNT_1_BIT,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                            AttachmentType::COLOR_ATTACHMENT,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            VK_IMAGE_VIEW_TYPE_2D_ARRAY,
                                            VK_FILTER_LINEAR,
                                            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    m_attachments[1] = Graphics::Attachment(static_cast<VkFormat>(m_depthFormat),
                                            VK_SAMPLE_COUNT_1_BIT,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                            AttachmentType::DEPTH_ATTACHMENT,
                                            VK_IMAGE_ASPECT_DEPTH_BIT,
                                            VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    // Depdencies
    m_dependencies.resize(2);

    m_dependencies[0] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    m_dependencies[1] = Graphics::SubPassDependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

   
    m_isResizeable = false;
   
}
void VarianceShadowPass::setup_uniforms() {

    m_device->create_descriptor_pool(
        m_descriptorPool, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS);
    m_descriptors.resize(RenderPass::frames.size());

    // GLOBAL SET
    VkDescriptorSetLayoutBinding camBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    VkDescriptorSetLayoutBinding sceneBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    VkDescriptorSetLayoutBinding shadowBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2); // ShadowMaps
    VkDescriptorSetLayoutBinding ssaoBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3); // SSAO
    VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBufferBinding, shadowBinding, ssaoBinding};
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 4);

    // PER-OBJECT SET
    VkDescriptorSetLayoutBinding objectBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0);
    VkDescriptorSetLayoutBinding materialBufferBinding = Init::descriptorset_layout_binding(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        1);
    VkDescriptorSetLayoutBinding objectBindings[] = {objectBufferBinding, materialBufferBinding};
    m_descriptorPool.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, objectBindings, 2);

    for (size_t i = 0; i < RenderPass::frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&RenderPass::frames[i].uniformBuffers[0],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(
            &RenderPass::frames[i].uniformBuffers[0],
            sizeof(SceneUniforms),
            Utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_device->get_GPU()),
            &m_descriptors[i].globalDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1);

        // Per-object
        m_descriptorPool.allocate_descriptor_set(
            DescriptorLayoutType::OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorPool.set_descriptor_write(&RenderPass::frames[i].uniformBuffers[1],
                                              sizeof(ObjectUniforms),
                                              0,
                                              &m_descriptors[i].objectDescritor,
                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                              0);
        m_descriptorPool.set_descriptor_write(
            &RenderPass::frames[i].uniformBuffers[1],
            sizeof(MaterialUniforms),
            Utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_device->get_GPU()),
            &m_descriptors[i].objectDescritor,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            1);
    }
}
void VarianceShadowPass::setup_shader_passes() {

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
    // settings.blendAttachments       = {};

    ShaderPass* depthPass =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/shadows/vsm_geom.glsl");
    depthPass->settings = settings;
    depthPass->build_shader_stages();
    depthPass->build(m_handle, m_descriptorPool, m_extent);
    m_shaderPasses["shadowTri"] = depthPass;

    ShaderPass* depthLinePass =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/shadows/vsm_line_geom.glsl");
    depthLinePass->settings             = settings;
    depthLinePass->settings.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    depthLinePass->settings.poligonMode = VK_POLYGON_MODE_LINE;
    depthLinePass->build_shader_stages();
    depthLinePass->build(m_handle, m_descriptorPool, m_extent);
    m_shaderPasses["shadowLine"] = depthLinePass;
}

void VarianceShadowPass::render(uint32_t frameIndex, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    VkCommandBuffer cmd = RenderPass::frames[frameIndex].commandBuffer;

    begin(cmd, presentImageIndex);

    VkViewport viewport = Init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdSetDepthBiasEnable(cmd, true);
    float depthBiasConstant = 0.0;
    float depthBiasSlope    = 0.0f;
    vkCmdSetDepthBias(cmd, depthBiasConstant, 0.0f, depthBiasSlope);

    int mesh_idx = 0;
    for (Mesh* m : scene->get_meshes())
    {
        if (m)
        {
            if (m->is_active() && m->get_cast_shadows() && m->get_num_geometries() > 0)
            {
                uint32_t objectOffset = RenderPass::frames[frameIndex].uniformBuffers[1].get_stride_size() * mesh_idx;
                uint32_t globalOffset = 0; // DEPENDENCY !!!!

                for (size_t i = 0; i < m->get_num_geometries(); i++)
                {

                    // Setup per object render state
                    IMaterial* mat = m->get_material(i);

                    ShaderPass* shaderPass =
                        mat->get_shaderpass_ID() != "hairstr" && mat->get_shaderpass_ID() != "hairstr2"
                            ? m_shaderPasses["shadowTri"]
                            : m_shaderPasses["shadowLine"];

                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->get_pipeline());

                    vkCmdSetDepthTestEnable(cmd, mat->get_parameters().depthTest);
                    vkCmdSetDepthWriteEnable(cmd, mat->get_parameters().depthWrite);
                    vkCmdSetCullMode(cmd,
                                     mat->get_parameters().faceCulling ? (VkCullModeFlags)mat->get_parameters().culling
                                                                       : VK_CULL_MODE_NONE);

                    // GLOBAL LAYOUT BINDING
                    uint32_t globalOffsets[] = {globalOffset, globalOffset};
                    vkCmdBindDescriptorSets(cmd,
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            shaderPass->get_layout(),
                                            0,
                                            1,
                                            &m_descriptors[frameIndex].globalDescritor.handle,
                                            2,
                                            globalOffsets);

                    // PER OBJECT LAYOUT BINDING
                    uint32_t objectOffsets[] = {objectOffset, objectOffset};
                    vkCmdBindDescriptorSets(cmd,
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            shaderPass->get_layout(),
                                            1,
                                            1,
                                            &m_descriptors[frameIndex].objectDescritor.handle,
                                            2,
                                            objectOffsets);

                    Geometry* g = m->get_geometry(i);
                    draw(cmd, g);
                }
            }
            mesh_idx++;
        }
    }

    end(cmd);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END