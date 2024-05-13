#include <engine/renderpasses/geometry_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void GeometryPass::init(VkDevice &device)
{

    std::array<VkAttachmentDescription, 3> attachmentsInfo = {};

    // Position attachment
    attachmentsInfo[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_attachments.push_back(
        Attachment(VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_VIEW_TYPE_2D));

    VkAttachmentReference posRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Normals attachment
    attachmentsInfo[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentsInfo[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_attachments.push_back(
        Attachment(VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_VIEW_TYPE_2D));

    VkAttachmentReference normalRef = init::attachment_reference(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Depth attachment
    attachmentsInfo[2].format = static_cast<VkFormat>(m_depthFormat);
    attachmentsInfo[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    m_attachments.push_back(
        Attachment(static_cast<VkFormat>(m_depthFormat),
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_IMAGE_ASPECT_DEPTH_BIT,
                   VK_IMAGE_VIEW_TYPE_2D));

    VkAttachmentReference depthRef = init::attachment_reference(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 2;
    std::array<VkAttachmentReference, 2> colorRefs = {posRef, normalRef};
    subpass.pColorAttachments = colorRefs.data();
    subpass.pDepthStencilAttachment = &depthRef;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
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
void GeometryPass::create_pipelines(VkDevice &device, DescriptorManager &descriptorManager)
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

    builder.multisampling = init::multisampling_state_create_info(VK_SAMPLE_COUNT_1_BIT);

    std::array<VkPipelineColorBlendAttachmentState, 2> colorBlendingAttchments = {init::color_blend_attachment_state(), init::color_blend_attachment_state()};
    builder.colorBlending = init::color_blend_create_info();
    builder.colorBlending.attachmentCount = 2;
    builder.colorBlending.pAttachments = colorBlendingAttchments.data();

    // Setup shaderpasses

    ShaderPass *geomPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/deferred.glsl");
    geomPass->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, true},
         {DescriptorLayoutType::TEXTURE_LAYOUT, false}};
    geomPass->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, true},
         {VertexAttributeType::UV, false},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};
    geomPass->settings.blending = true;

    ShaderPass::build_shader_stages(device, *geomPass);

    builder.build_pipeline_layout(device, descriptorManager, *geomPass);
    builder.build_pipeline(device, m_obj, *geomPass);

    m_shaderPasses["geometry"] = geomPass;
}
void GeometryPass::init_resources(VkDevice &device,
                                  VkPhysicalDevice &gpu,
                                  VmaAllocator &memory,
                                  VkQueue &gfxQueue,
                                  utils::UploadContext &uploadContext)
{
    m_attachments[0].image.create_sampler(
        device,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        0.0f,
        1.0f,
        false,
        1.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);
    m_attachments[1].image.create_sampler(
        device,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        0.0f,
        1.0f,
        false,
        1.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);

    m_attachments[2].image.create_sampler(
        device,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        0.0f,
        1.0f,
        false,
        1.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);

    set_attachment_clear_value({0.0, 0.0, 0.0, 1.0}, 0);
    set_attachment_clear_value({0.0, 0.0, 0.0, 1.0}, 1);
}
void GeometryPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd);

    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["geometry"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);

    int mesh_idx = 0;
    for (Mesh *m : scene->get_meshes())
    {
        if (m)
        {
            if (m->is_active() && m->get_cast_shadows() && m->get_num_geometries() > 0)
            {
                uint32_t objectOffset = frame.objectUniformBuffer.strideSize * mesh_idx;
                uint32_t globalOffset = 0;

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