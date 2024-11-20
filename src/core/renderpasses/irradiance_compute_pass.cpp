#include <engine/core/renderpasses/irradiance_compute_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void IrrandianceComputePass::setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                                               std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::Attachment(m_format,
                                          1,
                                          LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                          COLOR_ATTACHMENT,
                                          ASPECT_COLOR,
                                          TEXTURE_CUBE,
                                          FILTER_LINEAR,
                                          ADDRESS_MODE_CLAMP_TO_BORDER);

    // Depdencies
    dependencies.resize(1);

    dependencies[0] =
        Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_NONE);
}
void IrrandianceComputePass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 1);

    LayoutBinding panoramaTextureBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding auxBufferBinding(UniformDataType::UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {panoramaTextureBinding, auxBufferBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_captureDescriptorSet);
}
void IrrandianceComputePass::setup_shader_passes() {

    ShaderPass* converterPass =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/misc/irradiance_compute.glsl");
    converterPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    converterPass->settings.attributes             = {{POSITION_ATTRIBUTE, true},
                                                      {NORMAL_ATTRIBUTE, false},
                                                      {UV_ATTRIBUTE, false},
                                                      {TANGENT_ATTRIBUTE, false},
                                                      {COLOR_ATTRIBUTE, false}};

    converterPass->build_shader_stages();
    converterPass->build(m_handle, m_descriptorPool);

    m_shaderPasses["irr"] = converterPass;
}

void IrrandianceComputePass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_handle, m_framebuffers[0]);
    cmd.set_viewport(m_handle.extent);

    ShaderPass* shaderPass = m_shaderPasses["irr"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_captureDescriptorSet, 0, *shaderPass);

    Geometry* g = scene->get_skybox()->get_box();
    cmd.draw_geometry(*get_VAO(g));

    cmd.end_renderpass();
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

    const size_t BUFFER_SIZE = m_device->pad_uniform_buffer_size(sizeof(CaptureData));
    m_captureBuffer          = m_device->create_buffer_VMA(
        BUFFER_SIZE, BUFFER_USAGE_UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)BUFFER_SIZE);

    m_captureBuffer.upload_data(&capture, sizeof(CaptureData));

    m_descriptorPool.set_descriptor_write(
        &m_captureBuffer, BUFFER_SIZE, 0, &m_captureDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
}
void IrrandianceComputePass::connect_env_cubemap(Graphics::Image env) {
    m_descriptorPool.set_descriptor_write(&env, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_captureDescriptorSet, 0);
}

void IrrandianceComputePass::cleanup() {
    RenderPass::cleanup();
    m_captureBuffer.cleanup();
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END