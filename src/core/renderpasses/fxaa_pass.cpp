#include <engine/core/renderpasses/fxaa_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void FXAAPass::setup_attachments() {

    m_attachments.resize(1);

    m_attachments[0] =
        Graphics::Attachment(static_cast<VkFormat>(m_colorFormat),
                             VK_SAMPLE_COUNT_1_BIT,
                             m_isDefault ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                             VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                             AttachmentType::COLOR_ATTACHMENT,
                             VK_IMAGE_ASPECT_COLOR_BIT,
                             VK_IMAGE_VIEW_TYPE_2D,
                             VK_FILTER_LINEAR,
                             VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    m_attachments[0].isPresentImage = m_isDefault ? true : false;

    // Depdencies
    m_dependencies.resize(1);

    m_dependencies[0] = Graphics::SubPassDependency(
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
}
void FXAAPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_device->create_descriptor_pool(m_descriptorPool, 1, 1, 1, 1, 1);

    LayoutBinding outputTextureBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT,  {outputTextureBinding});

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

void FXAAPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer* cmd = currentFrame.commandBuffer;
    cmd->begin_renderpass(m_handle, m_framebuffers[presentImageIndex],m_extent, m_attachments);
    cmd->set_viewport(m_extent);

    ShaderPass* shaderPass = m_shaderPasses["fxaa"];

    cmd->bind_shaderpass(*shaderPass);
    cmd->bind_descriptor_set(m_imageDescriptorSet, 0, *shaderPass);

    Geometry* g = m_vignette->get_geometry();
    cmd->draw_geometry(*get_VAO(g));

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        cmd->draw_gui_data();

    cmd->end_renderpass();
}

void FXAAPass::connect_to_previous_images(std::vector<Image> images) {
    m_descriptorPool.set_descriptor_write(
        images[0].sampler, images[0].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptorSet, 0);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END