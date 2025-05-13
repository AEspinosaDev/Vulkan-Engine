#include <engine/render/passes/sky_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Render {

void SkyPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::AttachmentConfig(ColorFormatType::SRGBA_32F,
                                                1,
                                                LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                                COLOR_ATTACHMENT,
                                                ASPECT_COLOR,
                                                TEXTURE_2D,
                                                FILTER_LINEAR,
                                                ADDRESS_MODE_CLAMP_TO_EDGE);

    dependencies.resize(1);

    // dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[0]                 = Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[0].dependencyFlags = SUBPASS_DEPENDENCY_NONE;

    m_isResizeable = false;
}
void SkyPass::create_framebuffer() {

    m_interAttachments.resize(2);
    std::vector<Graphics::Image*> out1 = {&m_interAttachments[0]};
    std::vector<Graphics::Image*> out2 = {&m_interAttachments[1]};
    std::vector<Graphics::Image*> out3 = {m_outAttachments[0]};
    m_framebuffers[0]                  = m_device->create_framebuffer(m_renderpass, out1, m_imageExtent, m_framebufferImageDepth, 0);
    m_framebuffers[1]                  = m_device->create_framebuffer(m_renderpass, out2, m_imageExtent, m_framebufferImageDepth, 1);
    m_framebuffers[2]                  = m_device->create_framebuffer(m_renderpass, out3, m_imageExtent, m_framebufferImageDepth, 2);
}
void SkyPass::resize_attachments() {
    BaseGraphicPass::resize_attachments();

    // Update descriptor of previous framebuffer
    m_imageDescriptor.update(&m_interAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    m_imageDescriptor.update(&m_interAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
}
void SkyPass::setup_uniforms(std::vector<Graphics::Frame>& frames, const Render::Resources& shared) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 2);

    LayoutBinding LUTBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding SKYBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {LUTBinding, SKYBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_imageDescriptor);

    m_imageDescriptor.update(&m_interAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    m_imageDescriptor.update(&m_interAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
}
void SkyPass::setup_shader_passes() {

    GraphicShaderPass* ttPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/env/sky_tt_compute.glsl"));
    ttPass->settings.pushConstants          = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(Core::SkySettings) + sizeof(AerosolParams))};
    ttPass->settings.descriptorSetLayoutIDs = {{0, true}};
    ttPass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, false}, {COLOR_ATTRIBUTE, false}};

    ttPass->build_shader_stages();
    ttPass->build(m_descriptorPool);

    m_shaderPasses["tt"] = ttPass;

    GraphicShaderPass* skyPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/env/sky_generation.glsl"));
    skyPass->settings.pushConstants          = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(Core::SkySettings) + sizeof(AerosolParams))};
    skyPass->settings.descriptorSetLayoutIDs = ttPass->settings.descriptorSetLayoutIDs;
    skyPass->graphicSettings.attributes      = ttPass->graphicSettings.attributes;

    skyPass->build_shader_stages();
    skyPass->build(m_descriptorPool);

    m_shaderPasses["sky"] = skyPass;

    GraphicShaderPass* projPass =
        new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH("shaders/env/sky_projection.glsl"));
    projPass->settings.pushConstants          = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(int))};
    projPass->settings.descriptorSetLayoutIDs = ttPass->settings.descriptorSetLayoutIDs;
    projPass->graphicSettings.attributes      = ttPass->graphicSettings.attributes;

    projPass->build_shader_stages();
    projPass->build(m_descriptorPool);

    m_shaderPasses["proj"] = projPass;
}

void SkyPass::execute(const Render::RenderView& view, const Render::Resources& shared) {
    PROFILING_EVENT()
    // if (!scene->get_skybox())
    //     return;

    CommandBuffer cmd = view.commandBuffer;

    struct PassThroughSettings {
        Core::SkySettings sky;
        AerosolParams     aerosol;
    };

    PassThroughSettings passSettings;
    // passSettings.sky     = scene->get_skybox()->get_sky_settings();
    passSettings.aerosol = get_aerosol_params(passSettings.sky.aerosol);

    /* Transmittance LUT Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);
    ShaderPass* shaderPass = m_shaderPasses["tt"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &passSettings, sizeof(Core::SkySettings) + sizeof(AerosolParams));
    cmd.draw_geometry(*get_VAO(shared.vignette));
    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);

    /* Sky Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[1]);
    cmd.set_viewport(m_imageExtent);
    shaderPass = m_shaderPasses["sky"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &passSettings, sizeof(Core::SkySettings) + sizeof(AerosolParams));
    cmd.bind_descriptor_set(m_imageDescriptor, 0, *shaderPass);
    cmd.draw_geometry(*get_VAO(shared.vignette));
    cmd.end_renderpass(m_renderpass, m_framebuffers[1]);

    /* Sky Generation*/
    // ------------------------------------
    cmd.begin_renderpass(m_renderpass, m_framebuffers[2]);
    cmd.set_viewport(m_imageExtent);
    shaderPass = m_shaderPasses["proj"];
    cmd.bind_shaderpass(*shaderPass);
    int projectionType = passSettings.sky.useForIBL;
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &projectionType, sizeof(int));
    cmd.bind_descriptor_set(m_imageDescriptor, 0, *shaderPass);
    cmd.draw_geometry(*get_VAO(shared.vignette));
    cmd.end_renderpass(m_renderpass, m_framebuffers[2]);

    /* Sky is updated, set to sleep */
    if (passSettings.sky.updateType == UpdateType::ON_DEMAND)
        set_active(false);
}
SkyPass::AerosolParams SkyPass::get_aerosol_params(AerosolType type) {
    AerosolParams params = {};

    switch (type)
    {
    case AerosolType::BACKGROUND:
        params.aerosolAbsorptionCrossSection = Vec4(4.5517e-19, 5.9269e-19, 6.9143e-19, 8.5228e-19);
        params.aerosolScatteringCrossSection = Vec4(1.8921e-26, 1.6951e-26, 1.7436e-26, 2.1158e-26);
        params.aerosolBaseDensity            = 2.584e17;
        params.aerosolBackgroundDensity      = 2e6;
        break;

    case AerosolType::DESERT_DUST:
        params.aerosolAbsorptionCrossSection = Vec4(4.6758e-16, 4.4654e-16, 4.1989e-16, 4.1493e-16);
        params.aerosolScatteringCrossSection = Vec4(2.9144e-16, 3.1463e-16, 3.3902e-16, 3.4298e-16);
        params.aerosolBaseDensity            = 1.8662e18;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 2.0f;
        break;

    case AerosolType::MARITIME_CLEAN:
        params.aerosolAbsorptionCrossSection = Vec4(6.3312e-19, 7.5567e-19, 9.2627e-19, 1.0391e-18);
        params.aerosolScatteringCrossSection = Vec4(4.6539e-26, 2.721e-26, 4.1104e-26, 5.6249e-26);
        params.aerosolBaseDensity            = 2.0266e17;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 0.9f;
        break;

    case AerosolType::MARITIME_MINERAL:
        params.aerosolAbsorptionCrossSection = Vec4(6.9365e-19, 7.5951e-19, 8.2423e-19, 8.9101e-19);
        params.aerosolScatteringCrossSection = Vec4(2.3699e-19, 2.2439e-19, 2.2126e-19, 2.021e-19);
        params.aerosolBaseDensity            = 2.0266e17;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 2.0f;
        break;

    case AerosolType::POLAR_ANTARCTIC:
        params.aerosolAbsorptionCrossSection = Vec4(1.3399e-16, 1.3178e-16, 1.2909e-16, 1.3006e-16);
        params.aerosolScatteringCrossSection = Vec4(1.5506e-19, 1.809e-19, 2.3069e-19, 2.5804e-19);
        params.aerosolBaseDensity            = 2.3864e16;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 30.0f;
        break;

    case AerosolType::POLAR_ARCTIC:
        params.aerosolAbsorptionCrossSection = Vec4(1.0364e-16, 1.0609e-16, 1.0193e-16, 1.0092e-16);
        params.aerosolScatteringCrossSection = Vec4(2.1609e-17, 2.2759e-17, 2.5089e-17, 2.6323e-17);
        params.aerosolBaseDensity            = 2.3864e16;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 30.0f;
        break;

    case AerosolType::REMOTE_CONTINENTAL:
        params.aerosolAbsorptionCrossSection = Vec4(4.5307e-18, 5.0662e-18, 4.4877e-18, 3.7917e-18);
        params.aerosolScatteringCrossSection = Vec4(1.8764e-18, 1.746e-18, 1.6902e-18, 1.479e-18);
        params.aerosolBaseDensity            = 6.103e18;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 0.73f;
        break;

    case AerosolType::RURAL:
        params.aerosolAbsorptionCrossSection = Vec4(5.0393e-23, 8.0765e-23, 1.3823e-22, 2.3383e-22);
        params.aerosolScatteringCrossSection = Vec4(2.6004e-22, 2.4844e-22, 2.8362e-22, 2.7494e-22);
        params.aerosolBaseDensity            = 8.544e18;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 0.73f;
        break;

    case AerosolType::URBAN:
        params.aerosolAbsorptionCrossSection = Vec4(2.8722e-24, 4.6168e-24, 7.9706e-24, 1.3578e-23);
        params.aerosolScatteringCrossSection = Vec4(1.5908e-22, 1.7711e-22, 2.0942e-22, 2.4033e-22);
        params.aerosolBaseDensity            = 1.3681e20;
        params.aerosolBackgroundDensity      = 2e6;
        params.aerosolHeightScale            = 0.73f;
        break;

    default:
        // If type is unknown, set to default (0.0f values)
        break;
    }

    return params;
}

} // namespace Render

VULKAN_ENGINE_NAMESPACE_END