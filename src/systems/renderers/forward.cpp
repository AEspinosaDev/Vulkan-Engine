#include <engine/systems/renderers/forward.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void ForwardRenderer::on_before_render(Core::Scene* const scene) {
    update_enviroment(scene->get_skybox());
    BaseRenderer::on_before_render(scene);

    m_passes[FORWARD_PASS]->set_attachment_clear_value({m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 0);
    m_passes[FORWARD_PASS]->set_attachment_clear_value({m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 1);
}

void ForwardRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    BaseRenderer::on_after_render(renderResult, scene);

    if (m_updateShadows)
    {
        m_device->wait_idle();

        const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowQuality;

        m_passes[SHADOW_PASS]->set_extent({SHADOW_RES, SHADOW_RES});
        m_passes[SHADOW_PASS]->resize_attachments();

        m_updateShadows = false;

        m_passes[FORWARD_PASS]->link_input_attachments();
    }
}
void ForwardRenderer::create_passes() {

    // Create Passes and Attachments pool
    //--------------------------------
    m_passes.resize(8, nullptr);
    m_attachments.resize(11);

    // Main configs
    const Extent2D        DISPLAY_EXTENT = !m_headless ? m_window->get_extent() : m_headlessExtent;
    const ColorFormatType HDR_FORMAT     = m_settings.highDynamicPrecission == FloatPrecission::F16 ? SRGBA_16F : SRGBA_32F;
    const ColorFormatType DEPTH_FORMAT   = m_settings.depthPrecission == FloatPrecission::F16 ? DEPTH_16F : DEPTH_32F;
    const Extent2D        SHADOW_RES     = {(uint32_t)m_settings.shadowQuality, (uint32_t)m_settings.shadowQuality};

    // Arrange connectivity
    //--------------------------------

    Core::PassLinkage<0, 1> skyPassConfig         = {m_attachments, {}, {0}};
    Core::PassLinkage<1, 2> enviromentPassConfig  = {m_attachments, {0}, {1, 2}};
    Core::PassLinkage<0, 2> shadowPassConfig      = {m_attachments, {}, {3, 4}};
    Core::PassLinkage<3, 3> forwardPassConfig     = {m_attachments, {3, 1, 2}, {5, 6, 7}};
    Core::PassLinkage<2, 1> bloomPassConfig       = {m_attachments, {5, 6}, {8}};
    Core::PassLinkage<1, 1> FXAAPassConfig        = {m_attachments, {8}, {9}};
    Core::PassLinkage<1, 1> toneMappingPassConfig = {m_attachments, {9}, {10}};

    // Create passes
    //--------------------------------

    m_passes[SKY_PASS]        = std::make_shared<Core::SkyPass>(m_device, skyPassConfig, Extent2D{1024, 512});
    m_passes[ENVIROMENT_PASS] = std::make_shared<Core::EnviromentPass>(m_device, enviromentPassConfig);
    m_passes[SHADOW_PASS]     = std::make_shared<Core::VarianceShadowPass>(m_device, shadowPassConfig, SHADOW_RES, ENGINE_MAX_LIGHTS, DEPTH_FORMAT);
    m_passes[FORWARD_PASS] = std::make_shared<Core::ForwardPass>(m_device, forwardPassConfig, DISPLAY_EXTENT, HDR_FORMAT, DEPTH_FORMAT, m_settings.samplesMSAA);
    m_passes[BLOOM_PASS]   = std::make_shared<Core::BloomPass>(m_device, bloomPassConfig, DISPLAY_EXTENT);

    m_passes[FXAA_PASS] = std::make_shared<Core::PostProcessPass<1, 1>>(
        m_device, FXAAPassConfig, DISPLAY_EXTENT, HDR_FORMAT, GET_RESOURCE_PATH("shaders/aa/fxaa.glsl"), "FXAA", false);

    m_passes[TONEMAPPIN_PASS] =
        std::make_shared<Core::TonemappingPass>(m_device, toneMappingPassConfig, DISPLAY_EXTENT, m_settings.displayColorFormat, !m_headless);

    m_passes[GUI_PASS] = std::make_shared<Core::GUIPass>(m_device, DISPLAY_EXTENT);

    // Optional logic (unchanged):
    if (m_settings.softwareAA != SoftwareAA::FXAA)
        m_passes[FXAA_PASS]->set_active(false);
    if (m_headless)
        m_passes[GUI_PASS]->set_active(false);
}

void ForwardRenderer::update_enviroment(Core::Skybox* const skybox) {
    if (skybox)
    {
        if (skybox->update_enviroment())
        {

            Core::ResourceManager::upload_skybox_data(m_device, skybox);
            const uint32_t HDRi_EXTENT       = skybox->get_sky_type() == EnviromentType::IMAGE_BASED_ENV ? skybox->get_enviroment_map()->get_size().height
                                                                                                         : skybox->get_sky_settings().resolution;
            const uint32_t IRRADIANCE_EXTENT = skybox->get_irradiance_resolution();

            get_pass<Core::EnviromentPass>(ENVIROMENT_PASS)->set_irradiance_resolution(IRRADIANCE_EXTENT);
            m_passes[ENVIROMENT_PASS]->set_active(true);
            if (skybox->get_sky_type() == EnviromentType::PROCEDURAL_ENV)
                m_passes[SKY_PASS]->set_active(true);

            // ONLY IF: framebuffers needs to be resized
            // -------------------------------------------------------------------
            if (m_passes[ENVIROMENT_PASS]->get_extent().height != HDRi_EXTENT ||
                get_pass<Core::EnviromentPass>(ENVIROMENT_PASS)->get_irradiance_resolution() != IRRADIANCE_EXTENT)
            {
                m_device->wait_idle();
                if (skybox->get_sky_type() == EnviromentType::PROCEDURAL_ENV)
                {
                    m_passes[SKY_PASS]->set_extent({HDRi_EXTENT * 2, HDRi_EXTENT});
                    m_passes[SKY_PASS]->resize_attachments();
                    m_passes[ENVIROMENT_PASS]->link_input_attachments();
                }

                m_passes[ENVIROMENT_PASS]->set_extent({HDRi_EXTENT, HDRi_EXTENT});
                m_passes[ENVIROMENT_PASS]->resize_attachments();

                m_passes[FORWARD_PASS]->link_input_attachments();
            }
        }
    }
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END