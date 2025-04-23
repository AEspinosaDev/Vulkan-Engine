#include <engine/systems/renderers/forward.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void ForwardRenderer::on_before_render(Core::Scene* const scene) {
    update_enviroment(scene->get_skybox());
    BaseRenderer::on_before_render(scene);

    m_passes[FORWARD_PASS]->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 0);
    m_passes[FORWARD_PASS]->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 1);
}

void ForwardRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    BaseRenderer::on_after_render(renderResult, scene);

    if (m_updateShadows)
    {
        m_device->wait();

        const uint32_t SHADOW_RES = (uint32_t)m_shadowQuality;

        m_passes[SHADOW_PASS]->set_extent({SHADOW_RES, SHADOW_RES});
        m_passes[SHADOW_PASS]->resize_attachments();

        m_updateShadows = false;

        m_passes[FORWARD_PASS]->link_input_attachments();
    }
}
void ForwardRenderer::create_passes() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    // Create Passes and Attachments pool
    //--------------------------------
    m_passes.resize(6, nullptr);
    m_attachments.resize(9, {});

    // Arrange connectivity
    //--------------------------------
    Core::BasePass::attachmentPool = m_attachments;

    Core::PassConfig<1, 2> enviromentPassConfig  = {{0}, {0, 1}};
    Core::PassConfig<0, 2> shadowPassConfig      = {{}, {2, 3}};
    Core::PassConfig<3, 3> forwardPassConfig     = {{2, 0, 1}, {4, 5, 6}};
    Core::PassConfig<2, 1> bloomPassConfig       = {{4, 5}, {7}};
    Core::PassConfig<1, 1> toneMappingPassConfig = {{7}, {8}};
    Core::PassConfig<1, 1> FXAAPassConfig        = {{8}, {}};
    toneMappingPassConfig.isDefault              = m_settings.softwareAA ? false : true;
    FXAAPassConfig.isDefault                     = m_settings.softwareAA;

    // Create passes
    //--------------------------------
    m_passes[ENVIROMENT_PASS] = new Core::EnviromentPass(m_device, enviromentPassConfig);
    m_passes[SHADOW_PASS]     = new Core::VarianceShadowPass(
        m_device, shadowPassConfig, {SHADOW_RES, SHADOW_RES}, ENGINE_MAX_LIGHTS, m_settings.depthFormat);

    m_passes[FORWARD_PASS] = new Core::ForwardPass(m_device,
                                                   forwardPassConfig,
                                                   m_window->get_extent(),
                                                   m_settings.colorFormat,
                                                   m_settings.depthFormat,
                                                   m_settings.samplesMSAA);

    m_passes[BLOOM_PASS] = new Core::BloomPass(m_device, bloomPassConfig, m_window->get_extent());

    m_passes[TONEMAPPIN_PASS] =
        new Core::TonemappingPass(m_device, toneMappingPassConfig, m_window->get_extent(), m_settings.colorFormat);

    m_passes[FXAA_PASS] = new Core::PostProcessPass<1, 1>(m_device,
                                                          FXAAPassConfig,
                                                          m_window->get_extent(),
                                                          m_settings.colorFormat,
                                                          ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl",
                                                          "FXAA");

    m_passes.resize(6, nullptr);

    // if (!m_settings.softwareAA)
    //     m_passes[FXAA_PASS]->set_active(false);
}

void ForwardRenderer::update_enviroment(Core::Skybox* const skybox) {
    if (skybox)
    {
        if (skybox->update_enviroment())
        {
            m_device->wait();
            Core::ResourceManager::upload_skybox_data(m_device, skybox);
            const uint32_t HDRi_EXTENT       = skybox->get_enviroment_map()->get_size().height;
            const uint32_t IRRADIANCE_EXTENT = skybox->get_irradiance_resolution();

            get_pass<Core::EnviromentPass*>(ENVIROMENT_PASS)->set_irradiance_resolution(IRRADIANCE_EXTENT);
            m_passes[ENVIROMENT_PASS]->set_active(true);
            m_passes[ENVIROMENT_PASS]->set_extent({HDRi_EXTENT, HDRi_EXTENT});
            m_passes[ENVIROMENT_PASS]->resize_attachments();

            m_passes[FORWARD_PASS]->link_input_attachments();
        }
    }
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END