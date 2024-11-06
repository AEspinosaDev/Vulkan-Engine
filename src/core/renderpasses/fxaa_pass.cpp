#include <engine/core/renderpasses/fxaa_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void FXAAPass::setup_attachments() {

    std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

    // Color attachment
    attachmentsInfo[0].format         = static_cast<VkFormat>(m_colorFormat);
    attachmentsInfo[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout =
        m_isDefault ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    ImageConfig colorAttachmentImageConfig{};
    colorAttachmentImageConfig.format = static_cast<VkFormat>(m_colorFormat);
    colorAttachmentImageConfig.usageFlags =
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SamplerConfig colorAttachmentSamplerConfig{};
    colorAttachmentSamplerConfig.filters            = VK_FILTER_LINEAR;
    colorAttachmentSamplerConfig.samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    Attachment _colorAttachment(
        colorAttachmentImageConfig, {VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D}, colorAttachmentSamplerConfig);
    _colorAttachment.isPresentImage = m_isDefault ? true : false;
    m_attachments.push_back(_colorAttachment);

    VkAttachmentReference colorRef = Init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorRef;

    // Depdencies
    std::array<VkSubpassDependency, 1> dependencies = {{}};

    dependencies[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass    = 0;
    dependencies[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;

    // Creation
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
    renderPassInfo.pAttachments    = attachmentsInfo.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies   = dependencies.data();

    if (vkCreateRenderPass(m_device->get_handle(), &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
}
void FXAAPass::setup_uniforms() {
    // Init and configure local descriptors
    m_device->create_descriptor_pool(m_descriptorPool, 1, 1, 1, 1, 1);

    VkDescriptorSetLayoutBinding outputTextureBinding =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding bindings[] = {outputTextureBinding};
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 1);

    m_descriptorPool.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_imageDescriptorSet);
}
void FXAAPass::setup_shader_passes() {

    ShaderPass* fxaaPass = new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl");
    fxaaPass->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true}};
    fxaaPass->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                 {VertexAttributeType::NORMAL, false},
                                                 {VertexAttributeType::UV, true},
                                                 {VertexAttributeType::TANGENT, false},
                                                 {VertexAttributeType::COLOR, false}};
    // fxaaPass->settings.blending = false;
    // fxaaPass->settings.blendAttachments = {};

    fxaaPass->build_shader_stages();
    fxaaPass->build(m_handle, m_descriptorPool, m_extent);

    m_shaderPasses["fxaa"] = fxaaPass;
}

void FXAAPass::render(uint32_t frameIndex, Scene* const scene, uint32_t presentImageIndex) {
    VkCommandBuffer cmd = RenderPass::frames[frameIndex].commandBuffer;

    begin(cmd, presentImageIndex);

    VkViewport viewport = Init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass* shaderPass = m_shaderPasses["fxaa"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->get_pipeline());
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            shaderPass->get_layout(),
                            0,
                            1,
                            &m_imageDescriptorSet.handle,
                            0,
                            VK_NULL_HANDLE);

    Geometry* g = m_vignette->get_geometry();
    draw(cmd, g);

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled && ImGui::GetDrawData())
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    }

    end(cmd);
}

void FXAAPass::connect_to_previous_images(std::vector<Image> images) {
    m_descriptorPool.set_descriptor_write(
        images[0].sampler, images[0].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptorSet, 0);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END