#include <engine/renderpasses/shadowpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void ShadowPass::init(VkDevice &device)
{
    VkAttachmentDescription depthAttachment = init::attachment_description(static_cast<VkFormat>(m_depthFormat),
                                                                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT, false);
    m_attachments.push_back(Attachment({depthAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY}));

    VkSubpassDependency earlyDepthDep = init::subpass_dependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                                 VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    earlyDepthDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDependency lateDepthDep = init::subpass_dependency(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 0, VK_SUBPASS_EXTERNAL);
    lateDepthDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkAttachmentReference depthRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    VkSubpassDescription depthSubpass = init::subpass_description(0, VK_NULL_HANDLE, depthRef);

    m_isResizeable = false; // Always the same

    std::vector<VkSubpassDescription> subpasses = {depthSubpass};
    std::vector<VkSubpassDependency> dependencies = {earlyDepthDep, lateDepthDep};

    std::vector<VkAttachmentDescription> attachmentsSubDescriptions;
    attachmentsSubDescriptions.reserve(m_attachments.size());
    for (Attachment &attachment : m_attachments)
    {
        attachmentsSubDescriptions.push_back(attachment.description.description);
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)attachmentsSubDescriptions.size();
    renderPassInfo.pAttachments = attachmentsSubDescriptions.data();
    renderPassInfo.subpassCount = (uint32_t)subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_obj) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
    // Build renderpass
    // build(device, {depthSubpass}, {earlyDepthDep, lateDepthDep});
}
void ShadowPass::init_shaderpasses(VkDevice &device, DescriptorManager &descriptorManager)
{
    PipelineBuilder builder;

    // Default geometry assembly values
    builder.vertexInputInfo = init::vertex_input_state_create_info();
    builder.inputAssembly = init::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    auto bindingDescription = Vertex::getBindingDescription();
    builder.vertexInputInfo.vertexBindingDescriptionCount = 1;
    builder.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

    // Viewport
    builder.viewport.x = 0.0f;
    builder.viewport.y = 0.0f;
    builder.viewport.minDepth = 0.0f;
    builder.viewport.maxDepth = 1.0f;
    builder.scissor.offset = {0, 0};
    builder.viewport.width = (float)m_extent.width;
    builder.viewport.height = (float)m_extent.height;
    builder.scissor.extent = m_extent;
    builder.rasterizer.depthBiasEnable = VK_TRUE;

    builder.rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

    builder.depthStencil = init::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS);

    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    builder.colorBlending.attachmentCount = 0;

    // DEPTH PASS
    ShaderPass *depthPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/shadows.glsl");
    depthPass->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                  {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                  {DescriptorLayoutType::TEXTURE_LAYOUT, false}};
    depthPass->settings.attributes = {{VertexAttributeType::POSITION, true},
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
void ShadowPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t framebufferIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd);

    RenderPass::set_viewport(cmd, m_extent);

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