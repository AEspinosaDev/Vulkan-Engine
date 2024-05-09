#include <engine/renderpasses/forward_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void ForwardPass::init(VkDevice &device)
{
    bool multisampled = m_samples > VK_SAMPLE_COUNT_1_BIT;

    std::array<VkAttachmentDescription, 3> attachmentsInfo = {};

    // Color attachment
    attachmentsInfo[0].format = static_cast<VkFormat>(m_colorFormat);
    attachmentsInfo[0].samples = m_samples;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_attachments.push_back(
        Attachment(static_cast<VkFormat>(m_colorFormat),
                   VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_VIEW_TYPE_2D,
                   m_samples));

    VkAttachmentReference colorRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Resolve attachment
    attachmentsInfo[1].format = static_cast<VkFormat>(m_colorFormat);
    attachmentsInfo[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    Attachment resolveAttachment(static_cast<VkFormat>(m_colorFormat),
                                 VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_IMAGE_ASPECT_COLOR_BIT);
    resolveAttachment.isPresentImage = true;
    m_attachments.push_back(resolveAttachment);

    VkAttachmentReference resolveRef = init::attachment_reference(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Depth attachment
    attachmentsInfo[2].format = static_cast<VkFormat>(m_depthFormat);
    attachmentsInfo[2].samples = m_samples;
    attachmentsInfo[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    m_attachments.push_back(
        Attachment(static_cast<VkFormat>(m_depthFormat),
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                   VK_IMAGE_ASPECT_DEPTH_BIT,
                   VK_IMAGE_VIEW_TYPE_2D,
                   m_samples));

    VkAttachmentReference depthRef = init::attachment_reference(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;
    subpass.pResolveAttachments = &resolveRef;

    // Depdencies
    std::array<VkSubpassDependency, 2> dependencies = {};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Creation
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
    renderPassInfo.pAttachments = attachmentsInfo.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_obj) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
}
void ForwardPass::create_pipelines(VkDevice &device, DescriptorManager &descriptorManager)
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

    builder.rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

    builder.depthStencil = init::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS);

    builder.multisampling = init::multisampling_state_create_info(m_samples);

    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    builder.dynamicStates.push_back(VK_DYNAMIC_STATE_CULL_MODE);

    // Setup shaderpasses
    m_shaderPasses["unlit"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/unlit.glsl");
    m_shaderPasses["unlit"]->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, true},
         {DescriptorLayoutType::TEXTURE_LAYOUT, false}};
    m_shaderPasses["unlit"]->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, false},
         {VertexAttributeType::UV, false},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};

    m_shaderPasses["phong"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/phong.glsl");
    m_shaderPasses["phong"]->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, true},
         {DescriptorLayoutType::TEXTURE_LAYOUT, true}};
    m_shaderPasses["phong"]->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, true},
         {VertexAttributeType::UV, true},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};

    m_shaderPasses["physical"] = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/physically_based.glsl");
    m_shaderPasses["physical"]->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, true},
         {DescriptorLayoutType::TEXTURE_LAYOUT, true}};
    m_shaderPasses["physical"]->settings.attributes =
        {{VertexAttributeType::POSITION, true},
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
void ForwardPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd, presentImageIndex);

    // Viewport setup
    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // vkCmdSetDepthTestEnable(cmd, true);
    // vkCmdSetDepthWriteEnable(cmd, true);

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

                        // Setup per object render state
                        vkCmdSetDepthTestEnable(cmd, mat->get_parameters().depthTest);
                        vkCmdSetDepthWriteEnable(cmd, mat->get_parameters().depthWrite);
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

    //Draw gui contents
    if (m_gui)
        m_gui->upload_draw_data(cmd);

    end(cmd);
}

VULKAN_ENGINE_NAMESPACE_END