#include <engine/core/renderpasses/composition_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void CompositionPass::init(VkDevice &device)
{

    std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

    // Color attachment
    attachmentsInfo[0].format = static_cast<VkFormat>(m_colorFormat);
    attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = m_fxaa ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    Attachment _colorAttachment(
        static_cast<VkFormat>(m_colorFormat),
        m_fxaa ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
    _colorAttachment.isPresentImage = m_fxaa ? false : true;
    m_attachments.push_back(_colorAttachment);

    VkAttachmentReference colorRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    // Depdencies
    std::array<VkSubpassDependency, 1> dependencies = {{}};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;

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
void CompositionPass::create_pipelines(VkDevice &device, DescriptorManager &descriptorManager)
{

    ShaderPass *compPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/composition.glsl");
    compPass->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, true},
         {DescriptorLayoutType::G_BUFFER_LAYOUT, true}};
    compPass->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, false},
         {VertexAttributeType::UV, true},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};
    compPass->settings.blending = true;

    ShaderPass::build_shader_stages(device, *compPass);

    ShaderPass::build(device, m_obj, descriptorManager, m_extent, *compPass);

    m_shaderPasses["composition"] = compPass;

    // Allocate to global manager the gBuffer descritpro
    descriptorManager.allocate_descriptor_set(DescriptorLayoutType::G_BUFFER_LAYOUT, &m_GBufferDescriptor);
}
void CompositionPass::init_resources(VkDevice &device, VkPhysicalDevice &gpu, VmaAllocator &memory, VkQueue &gfxQueue, utils::UploadContext &uploadContext)
{
    const size_t BUFFER_SIZE = utils::pad_uniform_buffer_size(sizeof(Vec4), gpu);
    m_uniformBuffer.init(memory, BUFFER_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)BUFFER_SIZE);

    m_attachments[0].image.create_sampler(
        device,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        0.0f,
        1.0f,
        false,
        1.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
}

void CompositionPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd, presentImageIndex);

    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["composition"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);

    // GLOBAL LAYOUT BINDING
    uint32_t globalOffset = 0;
    uint32_t globalOffsets[] = {globalOffset, globalOffset};
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1, &frame.globalDescriptor.descriptorSet, 2, globalOffsets);
    // G BUFFER LAYOUT
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 3, 1, &m_GBufferDescriptor.descriptorSet, 0, VK_NULL_HANDLE);
    Geometry::draw(cmd, m_vignette->get_geometry());

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    end(cmd);
}

void CompositionPass::set_g_buffer(Image position, Image normals, Image albedo, Image material, DescriptorManager &descriptorManager)
{
    m_Gbuffer.resize(4);

    m_Gbuffer[0] = position;
    m_Gbuffer[1] = normals;
    m_Gbuffer[2] = albedo;
    m_Gbuffer[3] = material;

    descriptorManager.set_descriptor_write(m_Gbuffer[0].sampler, m_Gbuffer[0].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_GBufferDescriptor, 0);
    descriptorManager.set_descriptor_write(m_Gbuffer[1].sampler, m_Gbuffer[1].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_GBufferDescriptor, 1);
    descriptorManager.set_descriptor_write(m_Gbuffer[2].sampler, m_Gbuffer[2].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_GBufferDescriptor, 2);
    descriptorManager.set_descriptor_write(m_Gbuffer[3].sampler, m_Gbuffer[3].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_GBufferDescriptor, 3);
    descriptorManager.set_descriptor_write(&m_uniformBuffer, sizeof(Vec4), 0, &m_GBufferDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4);
}
void CompositionPass::update_uniforms(VmaAllocator &memory)
{
    struct AuxUniforms
    {
        Vec4 data;
    };
    AuxUniforms aux;
    aux.data = {m_outputType, 0.0f, 0.0f, 0.0f};
    m_uniformBuffer.upload_data(memory, &aux.data, sizeof(Vec4));
}

void CompositionPass::cleanup(VkDevice &device, VmaAllocator &memory)
{
    RenderPass::cleanup(device, memory);
    m_uniformBuffer.cleanup(memory);
}

void CompositionPass::update(VkDevice &device, VmaAllocator &memory, Swapchain *swp)
{
    RenderPass::update(device, memory, swp);
    m_attachments[0].image.create_sampler(
        device,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        0.0f,
        1.0f,
        false,
        1.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
}
VULKAN_ENGINE_NAMESPACE_END