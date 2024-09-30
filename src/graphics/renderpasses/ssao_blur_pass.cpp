#include <engine/graphics/renderpasses/ssao_blur_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void SSAOBlurPass::init(VkDevice &device)
{
    std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

    attachmentsInfo[0].format = VK_FORMAT_R8_UNORM;
    attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_attachments.push_back(
        Attachment(VK_FORMAT_R8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_VIEW_TYPE_2D));

    VkAttachmentReference ref = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;

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
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_obj) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
}

void SSAOBlurPass::create_descriptors(VkDevice &device, VkPhysicalDevice &gpu, VmaAllocator &memory, uint32_t framesPerFlight)
{

    // Init and configure local descriptors
    m_descriptorManager.init(device);
    m_descriptorManager.create_pool(1, 1, 1, 1, 1);

    VkDescriptorSetLayoutBinding ssaoTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, &ssaoTextureBinding, 1);

    m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptorSet);
}

void SSAOBlurPass::create_pipelines(VkDevice &device, DescriptorManager &descriptorManager)
{

    // DEPTH PASS
    ShaderPass *ssaoPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/ssao_blur.glsl");
    ssaoPass->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, false},
         {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
    ssaoPass->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, false},
         {VertexAttributeType::UV, true},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};

    ShaderPass::build_shader_stages(device, *ssaoPass);

    ShaderPass::build(device, m_obj, m_descriptorManager, m_extent, *ssaoPass);

    m_shaderPasses["ssaoBlur"] = ssaoPass;
}
void SSAOBlurPass::init_resources(VkDevice &device,
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
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

    m_descriptorManager.set_descriptor_write(m_ssao.sampler, m_ssao.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 0);
}
void SSAOBlurPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{

    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd, presentImageIndex);

    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["ssaoBlur"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1, &m_descriptorSet.descriptorSet, 0, VK_NULL_HANDLE);
    Geometry::draw(cmd, m_vignette->get_geometry());

    end(cmd);
}

void SSAOBlurPass::cleanup(VkDevice &device, VmaAllocator &memory)
{
    RenderPass::cleanup(device, memory);
    m_descriptorManager.cleanup();
}

void SSAOBlurPass::update(VkDevice &device, VmaAllocator &memory, Swapchain *swp)
{
    RenderPass::update(device, memory);
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

    m_descriptorManager.set_descriptor_write(m_ssao.sampler, m_ssao.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 0);
}

VULKAN_ENGINE_NAMESPACE_END