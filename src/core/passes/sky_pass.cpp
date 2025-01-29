#include <engine/core/passes/sky_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void SkyPass::setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::AttachmentInfo(ColorFormatType::SRGBA_32F,
                                              1,
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                              COLOR_ATTACHMENT,
                                              ASPECT_COLOR,
                                              TEXTURE_2D,
                                              FILTER_LINEAR,
                                              ADDRESS_MODE_CLAMP_TO_EDGE);

    dependencies.resize(1);

    // dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[0] = Graphics::SubPassDependency(
        STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[0].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
}
void SkyPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1,1,1, 1);

    LayoutBinding LUTBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {LUTBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_LUTdescriptor);
    m_descriptorPool.update_descriptor(
        &m_framebuffers[0].attachmentImages[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_LUTdescriptor, 0);
}
void SkyPass::setup_shader_passes() {

    GraphicShaderPass* ttPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/env/sky_tt_compute.glsl");
    ttPass->settings.descriptorSetLayoutIDs = {{0, true}};
    ttPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                               {NORMAL_ATTRIBUTE, false},
                                               {UV_ATTRIBUTE, true},
                                               {TANGENT_ATTRIBUTE, false},
                                               {COLOR_ATTRIBUTE, false}};

    ttPass->build_shader_stages();
    ttPass->build(m_descriptorPool);

    m_shaderPasses["tt"] = ttPass;

    GraphicShaderPass* skyPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/env/sky_generation.glsl");
    skyPass->settings.descriptorSetLayoutIDs = ttPass->settings.descriptorSetLayoutIDs;
    skyPass->graphicSettings.attributes      = ttPass->graphicSettings.attributes;

    skyPass->build_shader_stages();
    skyPass->build(m_descriptorPool);

    m_shaderPasses["sky"] = skyPass;
}

void SkyPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()
    CommandBuffer cmd = currentFrame.commandBuffer;
    Geometry*     g   = m_vignette->get_geometry();

    /* Transmittance LUT Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);
    ShaderPass* shaderPass = m_shaderPasses["tt"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.draw_geometry(*get_VAO(g));
    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);

    /* Transmittance LUT Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[1]);
    cmd.set_viewport(m_imageExtent);
    shaderPass = m_shaderPasses["sky"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_LUTdescriptor, 0, *shaderPass);
    cmd.draw_geometry(*get_VAO(g));
    cmd.end_renderpass(m_renderpass, m_framebuffers[1]);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END