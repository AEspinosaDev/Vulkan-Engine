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
void IrrandianceComputePass::setup_uniforms() {
    // Init and configure local descriptors
    m_device->create_descriptor_pool(m_descriptorPool, 1, 1, 1, 1, 1);

    VkDescriptorSetLayoutBinding panoramaTextureBinding =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding auxBufferBinding =
        Init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding bindings[] = {panoramaTextureBinding, auxBufferBinding};
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 2);

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

void IrrandianceComputePass::render(uint32_t frameIndex, Scene* const scene, uint32_t presentImageIndex) {

    VkCommandBuffer cmd = RenderPass::frames[frameIndex].commandBuffer;

    begin(cmd);

    VkViewport viewport = Init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass* shaderPass = m_shaderPasses["irr"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->get_pipeline());
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            shaderPass->get_layout(),
                            0,
                            1,
                            &m_captureDescriptorSet.handle,
                            0,
                            VK_NULL_HANDLE);

    draw(cmd, scene->get_skybox()->get_box());

    end(cmd);
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
    m_device->create_buffer(m_captureBuffer,
                            BUFFER_SIZE,
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VMA_MEMORY_USAGE_CPU_TO_GPU,
                            (uint32_t)BUFFER_SIZE);

    m_captureBuffer.upload_data(&capture, sizeof(CaptureData));

    m_descriptorPool.set_descriptor_write(
        &m_captureBuffer, BUFFER_SIZE, 0, &m_captureDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
}
void IrrandianceComputePass::connect_env_cubemap(Graphics::Image env) {
    m_descriptorPool.set_descriptor_write(
        env.sampler, env.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_captureDescriptorSet, 0);
}

void IrrandianceComputePass::cleanup() {
    RenderPass::cleanup();
    m_captureBuffer.cleanup();
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END