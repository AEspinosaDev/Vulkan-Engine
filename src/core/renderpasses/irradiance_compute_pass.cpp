#include <engine/core/renderpasses/irradiance_compute_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core
{

void IrrandianceComputePass::init()
{

    std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

    // Color attachment
    attachmentsInfo[0].format = static_cast<VkFormat>(m_format);
    attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ImageConfig colorAttachmentImageConfig{};
    colorAttachmentImageConfig.format =  static_cast<VkFormat>(m_format);
    colorAttachmentImageConfig.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    SamplerConfig colorAttachmentSamplerConfig{};
    colorAttachmentSamplerConfig.filters = VK_FILTER_LINEAR;
    colorAttachmentSamplerConfig.samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

    Attachment _colorAttachment(colorAttachmentImageConfig, {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE},
                                colorAttachmentSamplerConfig);
    _colorAttachment.isPresentImage = false;

    m_attachments.push_back(_colorAttachment);

    VkAttachmentReference colorRef = Init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

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

    if (vkCreateRenderPass(m_context->device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        new VKException("VkEngine exception: failed to create renderpass!");
    }

    m_initiatized = true;
}
void IrrandianceComputePass::create_descriptors()
{
    // Init and configure local descriptors
    m_descriptorManager.init(m_context->device);
    m_descriptorManager.create_pool(1, 1, 1, 1, 1);

    VkDescriptorSetLayoutBinding panoramaTextureBinding =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding auxBufferBinding =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding bindings[] = {panoramaTextureBinding, auxBufferBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 2);

    m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_captureDescriptorSet);
}
void IrrandianceComputePass::create_graphic_pipelines()
{

    ShaderPass *converterPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/misc/irradiance_compute.glsl");
    converterPass->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true}};
    converterPass->settings.attributes = {{VertexAttributeType::POSITION, true},
                                          {VertexAttributeType::NORMAL, false},
                                          {VertexAttributeType::UV, false},
                                          {VertexAttributeType::TANGENT, false},
                                          {VertexAttributeType::COLOR, false}};

    ShaderPass::build_shader_stages(m_context->device, *converterPass);

    ShaderPass::build(m_context->device, m_handle, m_descriptorManager, m_extent, *converterPass);

    m_shaderPasses["irr"] = converterPass;
}

void IrrandianceComputePass::render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{

    VkCommandBuffer cmd = m_context->frames[frameIndex].commandBuffer;

    begin(cmd);

    VkViewport viewport = Init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["irr"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1,
                            &m_captureDescriptorSet.handle, 0, VK_NULL_HANDLE);

    draw(cmd, scene->get_skybox()->get_box());

    end(cmd);
}

void IrrandianceComputePass::upload_data(uint32_t frameIndex, Scene *const scene)
{
    struct CaptureData
    {
        Mat4 proj{math::perspective(math::radians(90.0f), 1.0f, 0.1f, 10.0f)};
        Mat4 views[CUBEMAP_FACES] = {
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(-1.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(1.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, 1.0f, 0.0f), math::vec3(0.0f, 0.0f, 1.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f), math::vec3(0.0f, 0.0f, -1.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, 0.0f, 1.0f), math::vec3(0.0f, -1.0f, 0.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, 0.0f, -1.0f), math::vec3(0.0f, -1.0f, 0.0f))};
    };

    CaptureData capture{};

    const size_t BUFFER_SIZE = Utils::pad_uniform_buffer_size(sizeof(CaptureData), m_context->gpu);
    m_captureBuffer.init(m_context->memory, BUFFER_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)BUFFER_SIZE);

    m_captureBuffer.upload_data( &capture, sizeof(CaptureData));

    m_descriptorManager.set_descriptor_write(&m_captureBuffer, BUFFER_SIZE, 0, &m_captureDescriptorSet,
                                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
}
void IrrandianceComputePass::connect_env_cubemap(Graphics::Image env)
{
    m_descriptorManager.set_descriptor_write(env.sampler, env.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             &m_captureDescriptorSet, 0);
}

void IrrandianceComputePass::cleanup()
{
    RenderPass::cleanup();
    m_captureBuffer.cleanup();
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END