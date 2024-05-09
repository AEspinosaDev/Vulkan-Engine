#include <engine/renderpasses/shadow_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void ShadowPass::init(VkDevice &device)
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_obj) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
}
void ShadowPass::create_pipelines(VkDevice &device, DescriptorManager &descriptorManager)
{
    PipelineBuilder builder;

    // Default geometry assembly values
    builder.vertexInputInfo = init::vertex_input_state_create_info();
    builder.inputAssembly = init::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    auto bindingDescription = Vertex::getBindingDescription();
    builder.vertexInputInfo.vertexBindingDescriptionCount = 1;
    builder.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

    // Viewport
    builder.viewport = init::viewport(m_extent);
    builder.scissor.offset = {0, 0};
    builder.scissor.extent = m_extent;

    builder.rasterizer.depthBiasEnable = VK_TRUE;
    builder.rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

    builder.depthStencil = init::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS);

    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    builder.colorBlending.attachmentCount = 0;

    // DEPTH PASS
    ShaderPass *depthPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/shadows.glsl");
    depthPass->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, true},
         {DescriptorLayoutType::TEXTURE_LAYOUT, false}};
    depthPass->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, false},
         {VertexAttributeType::UV, false},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};

    //  builder.dynamicState = VK_TRUE;

    builder.multisampling = init::multisampling_state_create_info(VK_SAMPLE_COUNT_1_BIT);

    ShaderPass::build_shader_stages(device, *depthPass);

    builder.build_pipeline_layout(device, descriptorManager, *depthPass);
    builder.build_pipeline(device, m_obj, *depthPass);

    m_shaderPasses["shadow"] = depthPass;
}
void ShadowPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd);

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
                uint32_t objectOffset = frame.objectUniformBuffer.strideSize * mesh_idx;
                uint32_t globalOffset = 0; // DEPENDENCY !!!!

                for (size_t i = 0; i < m->get_num_geometries(); i++)
                {
                    // GLOBAL LAYOUT BINDING
                    uint32_t globalOffsets[] = {globalOffset, globalOffset};
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1, &frame.globalDescriptor.descriptorSet, 2, globalOffsets);
                    
                    // PER OBJECT LAYOUT BINDING
                    uint32_t objectOffsets[] = {objectOffset, objectOffset};
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 1, 1,
                                            &frame.objectDescriptor.descriptorSet, 2, objectOffsets);

                    if (m->get_geometry(i)->is_buffer_loaded())
                        Geometry::draw(cmd, m->get_geometry(i));
                }
            }
            mesh_idx++;
        }
    }

    end(cmd);
}

VULKAN_ENGINE_NAMESPACE_END