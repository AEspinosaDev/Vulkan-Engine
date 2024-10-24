#include <engine/core/renderpasses/panorama_conversion_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core
{

void PanroramaConverterPass::init()
{

    std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

    // Color attachment
    attachmentsInfo[0].format = VK_FORMAT_R8G8B8A8_SRGB;
    attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ImageConfig colorAttachmentImageConfig{};
    colorAttachmentImageConfig.format = VK_FORMAT_R8G8B8A8_SRGB;
    colorAttachmentImageConfig.usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    SamplerConfig colorAttachmentSamplerConfig{};
    colorAttachmentSamplerConfig.filters = VK_FILTER_LINEAR;
    colorAttachmentSamplerConfig.samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

    Attachment _colorAttachment(colorAttachmentImageConfig, {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE},
                                colorAttachmentSamplerConfig);
    _colorAttachment.isPresentImage = false;

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

    if (vkCreateRenderPass(m_context->device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        new VKException("VkEngine exception: failed to create renderpass!");
    }

    m_initiatized = true;
}
void PanroramaConverterPass::create_descriptors()
{
    // Init and configure local descriptors
    m_descriptorManager.init(m_context->device);
    m_descriptorManager.create_pool(1, 1, 1, 1, 1);

    VkDescriptorSetLayoutBinding panoramaTextureBinding =
        init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding bindings[] = {panoramaTextureBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 1);

    m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_panoramaDescriptorSet);
}
void PanroramaConverterPass::create_graphic_pipelines()
{

    ShaderPass *converterPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/panorama_converter.glsl");
    converterPass->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true}};
    converterPass->settings.attributes = {{VertexAttributeType::POSITION, true},
                                          {VertexAttributeType::NORMAL, false},
                                          {VertexAttributeType::UV, true},
                                          {VertexAttributeType::TANGENT, false},
                                          {VertexAttributeType::COLOR, false}};

    ShaderPass::build_shader_stages(m_context->device, *converterPass);

    ShaderPass::build(m_context->device, m_handle, m_descriptorManager, m_extent, *converterPass);

    m_shaderPasses["converter"] = converterPass;
}

void PanroramaConverterPass::render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{
    if (!scene->get_skybox())
        return;

    if (!scene->get_skybox()->update_enviroment())
        return;

    VkCommandBuffer cmd = m_context->frames[frameIndex].commandBuffer;

    begin(cmd);

    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["converter"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1,
                            &m_panoramaDescriptorSet.handle, 0, VK_NULL_HANDLE);

    Geometry *g = m_vignette->get_geometry();
    draw(cmd, g);

    end(cmd);

    scene->get_skybox()->set_update_enviroment(false);
}

void PanroramaConverterPass::upload_data(uint32_t frameIndex, Scene *const scene)
{
    if (!scene->get_skybox())
        return;
    TextureHDR *envMap = scene->get_skybox()->get_enviroment_map();
    if (envMap && envMap->is_buffer_loaded())
    {

        if (m_panoramaDescriptorSet.bindings == 0 || envMap->is_dirty())
        {
            m_descriptorManager.set_descriptor_write(get_image(envMap)->sampler, get_image(envMap)->view,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_panoramaDescriptorSet,
                                                     0);
            envMap->set_dirty(false);
        }
    }
}
void PanroramaConverterPass::connect_to_previous_images(std::vector<Image> images)
{
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END