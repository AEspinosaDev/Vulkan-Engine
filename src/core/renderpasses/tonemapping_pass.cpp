#include <engine/core/passes/tonemapping_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void TonemappingPass::setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                                        std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::Attachment(m_colorFormat,
                                          1,
                                          m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          m_isDefault ? IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT
                                                      : IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                          COLOR_ATTACHMENT,
                                          ASPECT_COLOR,
                                          TEXTURE_2D,
                                          FILTER_LINEAR,
                                          ADDRESS_MODE_CLAMP_TO_BORDER);

    attachments[0].isPresentImage = m_isDefault ? true : false;

    // Depdencies
    dependencies.resize(1);

    dependencies[0] =
        Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_NONE);
}
void TonemappingPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 1);

    LayoutBinding outputTextureBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {outputTextureBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_imageDescriptorSet);
}
void TonemappingPass::setup_shader_passes() {

    GraphicShaderPass* fxaaPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, ENGINE_RESOURCES_PATH "shaders/misc/tonemapping.glsl");
    fxaaPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    fxaaPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                                 {NORMAL_ATTRIBUTE, false},
                                                 {UV_ATTRIBUTE, true},
                                                 {TANGENT_ATTRIBUTE, false},
                                                 {COLOR_ATTRIBUTE, false}};
    // fxaaPass->settings.blending = false;
    // fxaaPass->settings.blendAttachments = {};

    fxaaPass->build_shader_stages();
    fxaaPass->build(m_descriptorPool);

    m_shaderPasses["tonemapping"] = fxaaPass;
}

void TonemappingPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[presentImageIndex]);
    cmd.set_viewport(m_renderpass.extent);

    ShaderPass* shaderPass = m_shaderPasses["tonemapping"];

    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_imageDescriptorSet, 0, *shaderPass);

    Geometry* g = m_vignette->get_geometry();
    cmd.draw_geometry(*get_VAO(g));

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        cmd.draw_gui_data();

    cmd.end_renderpass();
}

void TonemappingPass::connect_to_previous_images(std::vector<Image> images) {
    m_descriptorPool.set_descriptor_write(&images[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptorSet, 0);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END