#include <engine/core/renderpasses/irradiance_compute_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void IrrandianceComputePass::setup_attachments() {

    m_attachments.resize(1);

    m_attachments[0] = Graphics::Attachment(static_cast<VkFormat>(m_format),
                                            VK_SAMPLE_COUNT_1_BIT,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                            AttachmentType::COLOR_ATTACHMENT,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            VK_IMAGE_VIEW_TYPE_CUBE,
                                            VK_FILTER_LINEAR,
                                            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    // Depdencies
    m_dependencies.resize(1);

    m_dependencies[0] = Graphics::SubPassDependency(
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
}
void IrrandianceComputePass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_device->create_descriptor_pool(m_descriptorPool, 1, 1, 1, 1, 1);

    LayoutBinding panoramaTextureBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    LayoutBinding auxBufferBinding(UniformDataType::UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, {panoramaTextureBinding, auxBufferBinding});

    m_descriptorPool.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_captureDescriptorSet);
}
void IrrandianceComputePass::setup_shader_passes() {

    ShaderPass* converterPass =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/misc/irradiance_compute.glsl");
    converterPass->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true}};
    converterPass->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                      {VertexAttributeType::NORMAL, false},
                                                      {VertexAttributeType::UV, false},
                                                      {VertexAttributeType::TANGENT, false},
                                                      {VertexAttributeType::COLOR, false}};

    converterPass->build_shader_stages();
    converterPass->build(m_handle, m_descriptorPool, m_extent);

    m_shaderPasses["irr"] = converterPass;
}

void IrrandianceComputePass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer* cmd = currentFrame.commandBuffer;
    cmd->begin_renderpass(m_handle, m_framebuffers[0], m_extent, m_attachments);
    cmd->set_viewport(m_extent);

    ShaderPass* shaderPass = m_shaderPasses["irr"];
    cmd->bind_shaderpass(*shaderPass);
    cmd->bind_descriptor_set(m_captureDescriptorSet, 0, *shaderPass);

    Geometry* g = scene->get_skybox()->get_box();
    cmd->draw_geometry(*get_VAO(g));

    cmd->end_renderpass();
}

void IrrandianceComputePass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    struct CaptureData {
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

    const size_t BUFFER_SIZE = Utils::pad_uniform_buffer_size(sizeof(CaptureData), m_device->get_GPU());
    m_captureBuffer          = m_device->create_buffer_VMA(
        BUFFER_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)BUFFER_SIZE);

    m_captureBuffer.upload_data(&capture, sizeof(CaptureData));

    m_descriptorPool.set_descriptor_write(
        &m_captureBuffer, BUFFER_SIZE, 0, &m_captureDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
}
void IrrandianceComputePass::connect_env_cubemap(Graphics::Image env) {
    m_descriptorPool.set_descriptor_write(
        &env, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_captureDescriptorSet, 0);
}

void IrrandianceComputePass::cleanup() {
    RenderPass::cleanup();
    m_captureBuffer.cleanup();
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END