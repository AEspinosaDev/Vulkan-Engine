#include <engine/graphics/renderpasses/fxaa_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void FXAAPass::init(VkDevice &device)
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
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    Attachment _colorAttachment(static_cast<VkFormat>(m_colorFormat),
                                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
    _colorAttachment.isPresentImage = true;
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
void FXAAPass::create_descriptors(VkDevice &device, VkPhysicalDevice &gpu, VmaAllocator &memory, uint32_t framesPerFlight)
{

    // Init and configure local descriptors
    m_descriptorManager.init(device);
    m_descriptorManager.create_pool(1, 1, 1, 1, 1);

    VkDescriptorSetLayoutBinding outputTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding bindings[] = {outputTextureBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 1);

    m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptorSet);
}
void FXAAPass::create_pipelines(VkDevice &device, DescriptorManager &descriptorManager)
{

    ShaderPass *fxaaPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/fxaa.glsl");
    fxaaPass->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true}};
    fxaaPass->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, false},
         {VertexAttributeType::UV, true},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};

    ShaderPass::build_shader_stages(device, *fxaaPass);

    ShaderPass::build(device, m_obj, m_descriptorManager, m_extent, *fxaaPass);

    m_shaderPasses["fxaa"] = fxaaPass;
}

void FXAAPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd, presentImageIndex);

    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["fxaa"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1, &m_descriptorSet.descriptorSet, 0, VK_NULL_HANDLE);
    Geometry::draw(cmd, m_vignette->get_geometry());

    // // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    end(cmd);
}

void FXAAPass::set_output_buffer(Image output)
{
    m_outputBuffer = output;

    m_descriptorManager.set_descriptor_write(m_outputBuffer.sampler, m_outputBuffer.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 0);
}

void FXAAPass::cleanup(VkDevice &device, VmaAllocator &memory)
{
    RenderPass::cleanup(device, memory);
    m_descriptorManager.cleanup();
}

VULKAN_ENGINE_NAMESPACE_END