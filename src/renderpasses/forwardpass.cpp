#include <engine/renderpasses/forwardpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void ForwardPass::init(VkDevice &device)
{
    bool multisampled = m_samples > VK_SAMPLE_COUNT_1_BIT;

    VkAttachmentDescription colorAttachment = init::attachment_description(static_cast<VkFormat>(m_colorFormat),
                                                                           multisampled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                                                           m_samples);
    m_attachments.push_back(Attachment({colorAttachment,
                                        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                        VK_IMAGE_ASPECT_COLOR_BIT}));

    if (multisampled)
    {
        VkAttachmentDescription resolveAttachment = init::attachment_description(static_cast<VkFormat>(m_colorFormat), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        m_attachments.push_back(Attachment({resolveAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT}));
    }

    VkAttachmentDescription depthAttachment = init::attachment_description(static_cast<VkFormat>(m_depthFormat),
                                                                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                                                           m_samples);
    m_attachments.push_back(Attachment({depthAttachment,
                                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                        VK_IMAGE_ASPECT_DEPTH_BIT}));

    VkSubpassDependency colorDep = init::subpass_dependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                            0,
                                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
    VkSubpassDependency depthDep = init::subpass_dependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                            0,
                                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

    VkAttachmentReference colorRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkAttachmentReference depthRef = init::attachment_reference(static_cast<uint32_t>(m_attachments.size()) - 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    VkSubpassDescription defaultSubpass = init::subpass_description(1, &colorRef, depthRef);
    if (multisampled)
    {
        VkAttachmentReference resolveRef = init::attachment_reference(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        defaultSubpass.pResolveAttachments = &resolveRef;
    }

    std::vector<VkSubpassDescription> subpasses = {defaultSubpass};
    std::vector<VkSubpassDependency> dependencies = {colorDep, depthDep};

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
    // // Build renderpass
    // build(device, {defaultSubpass}, {colorDep, depthDep});
}
void ForwardPass::init_shaderpasses(VkDevice &device, DescriptorManager &descriptorManager)
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
    builder.viewport.width = m_extent.width;
    builder.viewport.height = m_extent.height;
    builder.viewport.minDepth = 0.0f;
    builder.viewport.maxDepth = 1.0f;
    builder.scissor.offset = {0, 0};
    builder.scissor.extent = m_extent;

    builder.rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

    builder.depthStencil = init::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS);

    builder.multisampling = init::multisampling_state_create_info(m_samples);

    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_CULL_MODE);

    // Setup shaderpasses
    m_shaderPasses["unlit"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/unlit.glsl");
    m_shaderPasses["unlit"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                {DescriptorLayoutType::TEXTURE_LAYOUT, false}};
    m_shaderPasses["unlit"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                    {VertexAttributeType::NORMAL, false},
                                                    {VertexAttributeType::UV, false},
                                                    {VertexAttributeType::TANGENT, false},
                                                    {VertexAttributeType::COLOR, false}};

    m_shaderPasses["phong"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/phong.glsl");
    m_shaderPasses["phong"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                {DescriptorLayoutType::TEXTURE_LAYOUT, true}};
    m_shaderPasses["phong"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                    {VertexAttributeType::NORMAL, true},
                                                    {VertexAttributeType::UV, true},
                                                    {VertexAttributeType::TANGENT, false},
                                                    {VertexAttributeType::COLOR, false}};

    m_shaderPasses["physical"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/physically_based.glsl");
    m_shaderPasses["physical"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
                                                                   {DescriptorLayoutType::OBJECT_LAYOUT, true},
                                                                   {DescriptorLayoutType::TEXTURE_LAYOUT, true}};
    m_shaderPasses["physical"]->settings.attributes = {{VertexAttributeType::POSITION, true},
                                                       {VertexAttributeType::NORMAL, true},
                                                       {VertexAttributeType::UV, true},
                                                       {VertexAttributeType::TANGENT, true},
                                                       {VertexAttributeType::COLOR, false}};

    for (auto pair : m_shaderPasses)
    {
        ShaderPass *pass = pair.second;

        ShaderPass::build_shader_stages(device, *pass);

        builder.build_pipeline_layout(device, descriptorManager, *pass);
        builder.build_pipeline(device, m_obj, *pass);
    }
}
void ForwardPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t framebufferIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd, framebufferIndex);

    RenderPass::set_viewport(cmd, m_extent);

    vkCmdSetDepthTestEnable(cmd, true);
    vkCmdSetDepthWriteEnable(cmd, true);

    if (scene->get_active_camera() && scene->get_active_camera()->is_active())
    {

        unsigned int mesh_idx = 0;
        for (Mesh *m : scene->get_meshes())
        {
            if (m)
            {
                if (m->is_active() &&                                                                     // Check if is active
                    m->get_num_geometries() > 0 &&                                                        // Check if has geometry
                    m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum())) // Check if is inside frustrum
                {
                    // Offset calculation
                    uint32_t objectOffset = frame.objectUniformBuffer.strideSize * mesh_idx;
                    uint32_t globalOffset = 0;

                    for (size_t i = 0; i < m->get_num_geometries(); i++)
                    {
                        Geometry *g = m->get_geometry(i);

                        Material *mat = m->get_material(g->get_material_ID());

                        // // Setup per object render state
                        // if (m_settings.depthTest)
                        //     vkCmdSetDepthTestEnable(cmd, mat->get_parameters().depthTest);
                        // if (m_settings.depthWrite)
                        //     vkCmdSetDepthWriteEnable(cmd, mat->get_parameters().depthWrite);
                        vkCmdSetCullMode(cmd, mat->get_parameters().faceCulling ? (VkCullModeFlags)mat->get_parameters().culling : VK_CULL_MODE_NONE);
                        ShaderPass *shaderPass = m_shaderPasses[mat->get_shaderpass_ID()];

                        // Bind pipeline
                        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);

                        // GLOBAL LAYOUT BINDING
                        uint32_t globalOffsets[] = {globalOffset, globalOffset};
                        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1, &frame.globalDescriptor.descriptorSet, 2, globalOffsets);

                        // PER OBJECT LAYOUT BINDING
                        uint32_t objectOffsets[] = {objectOffset, objectOffset};
                        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 1, 1, &frame.objectDescriptor.descriptorSet, 2, objectOffsets);

                        // TEXTURE LAYOUT BINDING
                        if (shaderPass->settings.descriptorSetLayoutIDs[DescriptorLayoutType::TEXTURE_LAYOUT])
                            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 2, 1, &mat->get_texture_descriptor().descriptorSet, 0, nullptr);

                        if (g->is_buffer_loaded())
                            Geometry::draw(cmd, g);
                    }
                }
            }
            mesh_idx++;
        }
    }

    if (m_gui)
        m_gui->upload_draw_data(cmd);

    end(cmd);
}

VULKAN_ENGINE_NAMESPACE_END