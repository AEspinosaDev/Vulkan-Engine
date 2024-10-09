#include <engine/graphics/renderpasses/shadow_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void ShadowPass::init()
{
    std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

    attachmentsInfo[0].format = static_cast<VkFormat>(m_depthFormat);
    attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    m_attachments.push_back(
        Attachment(static_cast<VkFormat>(m_depthFormat),
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_IMAGE_ASPECT_DEPTH_BIT,
                   VK_IMAGE_VIEW_TYPE_2D_ARRAY));

    VkAttachmentReference depthRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthRef;

    // Depdencies
    std::array<VkSubpassDependency, 2> dependencies = {};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Creation
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
    renderPassInfo.pAttachments = attachmentsInfo.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(m_context->device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_isResizeable = false;

    m_initiatized = true;
}
void ShadowPass::create_descriptors()
{

    m_descriptorManager.init(m_context->device);
    m_descriptorManager.create_pool(VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS);
    m_descriptors.resize(m_context->frames.size());

    // GLOBAL SET
    VkDescriptorSetLayoutBinding camBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding sceneBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding shadowBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2); // ShadowMaps
    VkDescriptorSetLayoutBinding ssaoBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);   // SSAO
    VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBufferBinding, shadowBinding, ssaoBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 4);

    // PER-OBJECT SET
    VkDescriptorSetLayoutBinding objectBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding materialBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding objectBindings[] = {objectBufferBinding, materialBufferBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, objectBindings, 2);

    for (size_t i = 0; i < m_context->frames.size(); i++)
    {
        // Global
        m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorManager.set_descriptor_write(&m_context->frames[i].uniformBuffers[0], sizeof(CameraUniforms), 0,
                                                 &m_descriptors[i].globalDescritor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);
        m_descriptorManager.set_descriptor_write(&m_context->frames[i].uniformBuffers[0], sizeof(SceneUniforms),
                                                 utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_context->gpu),
                                                 &m_descriptors[i].globalDescritor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);

        // Per-object
        m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::OBJECT_LAYOUT, &m_descriptors[i].objectDescritor);
        m_descriptorManager.set_descriptor_write(&m_context->frames[i].uniformBuffers[1], sizeof(ObjectUniforms), 0,
                                                 &m_descriptors[i].objectDescritor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);
        m_descriptorManager.set_descriptor_write(&m_context->frames[i].uniformBuffers[1], sizeof(MaterialUniforms),
                                                 utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_context->gpu),
                                                 &m_descriptors[i].objectDescritor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);
    }
}
void ShadowPass::create_pipelines()
{

    // DEPTH PASS
    ShaderPass *depthPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/shadows.glsl");
    depthPass->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    depthPass->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, false},
         {VertexAttributeType::UV, false},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};
    depthPass->settings.dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_DEPTH_BIAS,
        VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE};
    depthPass->settings.blending = false;
    depthPass->settings.blendAttachments = {};

    ShaderPass::build_shader_stages(m_context->device, *depthPass);

    ShaderPass::build(m_context->device, m_handle, m_descriptorManager, m_extent, *depthPass);

    m_shaderPasses["shadow"] = depthPass;
}

void ShadowPass::render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    VkCommandBuffer cmd = m_context->frames[frameIndex].commandBuffer;

    begin(cmd, presentImageIndex);

    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["shadow"];

    vkCmdSetDepthBiasEnable(cmd, true);
    float depthBiasConstant = 0.0;
    float depthBiasSlope = 0.0f;
    vkCmdSetDepthBias(
        cmd,
        depthBiasConstant,
        0.0f,
        depthBiasSlope);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline); // DEPENDENCY !!!!

    int mesh_idx = 0;
    for (Mesh *m : scene->get_meshes())
    {
        if (m)
        {
            if (m->is_active() && m->get_cast_shadows() && m->get_num_geometries() > 0)
            {
                uint32_t objectOffset = m_context->frames[frameIndex].uniformBuffers[1].strideSize * mesh_idx;
                uint32_t globalOffset = 0; // DEPENDENCY !!!!

                for (size_t i = 0; i < m->get_num_geometries(); i++)
                {
                    // GLOBAL LAYOUT BINDING
                    uint32_t globalOffsets[] = {globalOffset, globalOffset};
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1,
                                            &m_descriptors[frameIndex].globalDescritor.handle, 2, globalOffsets);

                    // PER OBJECT LAYOUT BINDING
                    uint32_t objectOffsets[] = {objectOffset, objectOffset};
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 1, 1,
                                            &m_descriptors[frameIndex].objectDescritor.handle, 2, objectOffsets);

                    Geometry *g = m->get_geometry(i);
                    draw(cmd, g);
                }
            }
            mesh_idx++;
        }
    }

    end(cmd);
}


VULKAN_ENGINE_NAMESPACE_END