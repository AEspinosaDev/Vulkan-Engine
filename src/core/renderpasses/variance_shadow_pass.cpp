#include <engine/core/renderpasses/variance_shadow_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void VarianceShadowPass::setup_attachments() {
    std::array<VkAttachmentDescription, 2> attachmentsInfo = {};

    attachmentsInfo[0].format         = static_cast<VkFormat>(m_format);
    attachmentsInfo[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ImageConfig VSMImageConfig{};
    VSMImageConfig.format     = static_cast<VkFormat>(m_format);
    VSMImageConfig.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    SamplerConfig depthAttachmentSamplereConfig{};
    depthAttachmentSamplereConfig.filters            = VK_FILTER_LINEAR;
    depthAttachmentSamplereConfig.samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    m_attachments.push_back(Attachment(
        VSMImageConfig, {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY}, depthAttachmentSamplereConfig));

    VkAttachmentReference VSMRef = Init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Depth attachment
    attachmentsInfo[1].format         = static_cast<VkFormat>(m_depthFormat);
    attachmentsInfo[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    ImageConfig depthAttachmentImageConfig{};
    depthAttachmentImageConfig.format     = static_cast<VkFormat>(m_depthFormat);
    depthAttachmentImageConfig.usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthAttachmentImageConfig.samples    = VK_SAMPLE_COUNT_1_BIT;
    m_attachments.push_back(
        Attachment(depthAttachmentImageConfig, {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY}, {}));

    VkAttachmentReference depthRef =
        Init::attachment_reference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass    = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &VSMRef;
    subpass.pDepthStencilAttachment = &depthRef;

    // Depdencies
    std::array<VkSubpassDependency, 2> dependencies = {};

    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask   = 0;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass      = 0;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask   = 0;
    dependencies[1].dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Creation
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
    renderPassInfo.pAttachments    = attachmentsInfo.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies   = dependencies.data();

    if (vkCreateRenderPass(m_device->get_handle(), &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_isResizeable = false;

    m_initiatized = true;
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

    // ShaderPass* depthLinePass =
    //     new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/shadows/vsm_line_geom.glsl");
    // depthLinePass->settings             = settings;
    // depthLinePass->settings.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    // depthLinePass->settings.poligonMode = VK_POLYGON_MODE_LINE;
    // depthLinePass->build_shader_stages();
    // depthLinePass->build(m_handle, m_descriptorPool, m_extent);
    // m_shaderPasses["shadowLine"] = depthLinePass;
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