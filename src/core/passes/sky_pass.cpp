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

    m_isResizeable = false;
}
void SkyPass::create_framebuffer() {
    m_framebuffers[0] = m_device->create_framebuffer(m_renderpass, m_imageExtent, m_framebufferImageDepth, 0);
    m_framebuffers[1] = m_device->create_framebuffer(m_renderpass, m_imageExtent, m_framebufferImageDepth, 1);
    m_framebuffers[2] = m_device->create_framebuffer(m_renderpass, m_imageExtent, m_framebufferImageDepth, 2);
}
void SkyPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 2);

    LayoutBinding LUTBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding SKYBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {LUTBinding, SKYBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_imageDescriptor);

    m_descriptorPool.update_descriptor(
        &m_framebuffers[0].attachmentImages[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptor, 0);
    m_descriptorPool.update_descriptor(
        &m_framebuffers[1].attachmentImages[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptor, 1);
}
void SkyPass::setup_shader_passes() {

    GraphicShaderPass* ttPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/env/sky_tt_compute.glsl");
    ttPass->settings.pushConstants          = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(Core::SkySettings))};
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
    skyPass->settings.pushConstants          = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(Core::SkySettings))};
    skyPass->settings.descriptorSetLayoutIDs = ttPass->settings.descriptorSetLayoutIDs;
    skyPass->graphicSettings.attributes      = ttPass->graphicSettings.attributes;
    

    skyPass->build_shader_stages();
    skyPass->build(m_descriptorPool);

    m_shaderPasses["sky"] = skyPass;

    GraphicShaderPass* projPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/env/sky_projection.glsl");
    projPass->settings.pushConstants          = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(int))};
    projPass->settings.descriptorSetLayoutIDs = ttPass->settings.descriptorSetLayoutIDs;
    projPass->graphicSettings.attributes      = ttPass->graphicSettings.attributes;

    projPass->build_shader_stages();
    projPass->build(m_descriptorPool);

    m_shaderPasses["proj"] = projPass;
}

void SkyPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()
    if (!scene->get_skybox())
        return;

    CommandBuffer cmd = currentFrame.commandBuffer;

    SkySettings skySettings = scene->get_skybox()->get_sky_settings();

    /* Transmittance LUT Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);
    ShaderPass* shaderPass = m_shaderPasses["tt"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &skySettings, sizeof(SkySettings));
    cmd.draw_geometry(*get_VAO(BasePass::vignette));
    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);

    /* Sky Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[1]);
    cmd.set_viewport(m_imageExtent);
    shaderPass = m_shaderPasses["sky"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &skySettings, sizeof(SkySettings));
    cmd.bind_descriptor_set(m_imageDescriptor, 0, *shaderPass);
    cmd.draw_geometry(*get_VAO(BasePass::vignette));
    cmd.end_renderpass(m_renderpass, m_framebuffers[1]);

    /* Sky Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[2]);
    cmd.set_viewport(m_imageExtent);
    shaderPass = m_shaderPasses["proj"];
    cmd.bind_shaderpass(*shaderPass);
    int projectionType = skySettings.useForIBL;
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &projectionType, sizeof(int));
    cmd.bind_descriptor_set(m_imageDescriptor, 0, *shaderPass);
    cmd.draw_geometry(*get_VAO(BasePass::vignette));
    cmd.end_renderpass(m_renderpass, m_framebuffers[2]);
}

} // namespace Core

// struct Matrix4x4 {
//     float m[4][4];
// };

// struct EulerAngles {
//     float pitch; // Rotation around X-axis
//     float yaw;   // Rotation around Y-axis
//     float roll;  // Rotation around Z-axis
// };

// // Function to extract Euler angles from the view matrix
// EulerAngles extractEulerAnglesFromViewMatrix(const Matrix4x4& viewMatrix) {
//     // Extract the rotation matrix (upper-left 3x3)
//     float r00 = viewMatrix.m[0][0], r01 = viewMatrix.m[0][1], r02 = viewMatrix.m[0][2];
//     float r10 = viewMatrix.m[1][0], r11 = viewMatrix.m[1][1], r12 = viewMatrix.m[1][2];
//     float r20 = viewMatrix.m[2][0], r21 = viewMatrix.m[2][1], r22 = viewMatrix.m[2][2];

//     // Calculate yaw, pitch, and roll
//     float yaw = std::atan2(r10, r00); // Yaw (rotation around Y-axis)
//     float pitch = std::atan2(-r20, std::sqrt(r00 * r00 + r10 * r10)); // Pitch (rotation around X-axis)
//     float roll = std::atan2(r21, r22); // Roll (rotation around Z-axis)

//     // Return Euler angles
//     return { pitch, yaw, roll };
// }

VULKAN_ENGINE_NAMESPACE_END