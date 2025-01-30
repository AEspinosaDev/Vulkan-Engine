#include <engine/core/passes/postprocess_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void PostProcessPass::setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                        std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] =
        Graphics::AttachmentInfo(m_colorFormat,
                                 1,
                                 m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 m_isDefault ? IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT
                                             : IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                 COLOR_ATTACHMENT,
                                 ASPECT_COLOR,
                                 TEXTURE_2D,
                                 FILTER_LINEAR,
                                 ADDRESS_MODE_CLAMP_TO_EDGE);

    attachments[0].isDefault = m_isDefault ? true : false;

    // Depdencies
    if (!m_isDefault)
    {
        dependencies.resize(2);

        dependencies[0] = Graphics::SubPassDependency(
            STAGE_FRAGMENT_SHADER, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
        dependencies[0].srcAccessMask   = ACCESS_SHADER_READ;
        dependencies[0].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
        dependencies[1] =
            Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_FRAGMENT_SHADER, ACCESS_SHADER_READ);
        dependencies[1].srcAccessMask   = ACCESS_COLOR_ATTACHMENT_WRITE;
        dependencies[1].srcSubpass      = 0;
        dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[1].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
    } else
    {
        dependencies.resize(1);

        // dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[0] = Graphics::SubPassDependency(
            STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
        dependencies[0].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
    }
}
void PostProcessPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 1);

    LayoutBinding outputTextureBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {outputTextureBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_imageDescriptorSet);
}
void PostProcessPass::setup_shader_passes() {

    GraphicShaderPass* ppPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, m_shaderPath);
    ppPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    ppPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                               {NORMAL_ATTRIBUTE, false},
                                               {UV_ATTRIBUTE, true},
                                               {TANGENT_ATTRIBUTE, false},
                                               {COLOR_ATTRIBUTE, false}};

    ppPass->build_shader_stages();
    ppPass->build(m_descriptorPool);

    m_shaderPasses["pp"] = ppPass;
}

void PostProcessPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[m_isDefault ? presentImageIndex : 0]);
    cmd.set_viewport(m_imageExtent);

    ShaderPass* shaderPass = m_shaderPasses["pp"];

    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_imageDescriptorSet, 0, *shaderPass);

    cmd.draw_geometry(*get_VAO(BasePass::vignette));

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        cmd.draw_gui_data();

    cmd.end_renderpass(m_renderpass, m_framebuffers[m_isDefault ? presentImageIndex : 0]);
}

void PostProcessPass::link_previous_images(std::vector<Image> images) {
    m_descriptorPool.update_descriptor(&images[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptorSet, 0);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END