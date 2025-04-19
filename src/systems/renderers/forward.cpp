#include <engine/systems/renderers/forward.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void ForwardRenderer::on_before_render(Core::Scene* const scene) {
    update_enviroment(scene->get_skybox());
    BaseRenderer::on_before_render(scene);

    static_cast<Core::GraphicPass*>(m_passes[FORWARD_PASS])->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 0);
        static_cast<Core::GraphicPass*>(m_passes[FORWARD_PASS])->set_attachment_clear_value(
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

        connect_pass(m_passes[FORWARD_PASS]);
    }
}
void ForwardRenderer::create_passes() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    m_passes.resize(6, nullptr);
    // Enviroment Pass
    m_passes[ENVIROMENT_PASS] = new Core::EnviromentPass(m_device);

    // Shadow Pass
    m_passes[SHADOW_PASS] =
        new Core::VarianceShadowPass(m_device, {SHADOW_RES, SHADOW_RES}, ENGINE_MAX_LIGHTS, m_settings.depthFormat);

    // Forward Pass
    m_passes[FORWARD_PASS] = new Core::ForwardPass(
        m_device, m_window->get_extent(), SRGBA_32F, m_settings.depthFormat, m_settings.samplesMSAA, false);
    m_passes[FORWARD_PASS]->set_image_dependencies({Core::ImageDependency(SHADOW_PASS, 0, {0}),
                                                    Core::ImageDependency(ENVIROMENT_PASS, 0, {0}),
                                                    Core::ImageDependency(ENVIROMENT_PASS, 1, {0})});

    // Bloom Pass
    m_passes[BLOOM_PASS] = new Core::BloomPass(m_device, m_window->get_extent());
    m_passes[BLOOM_PASS]->set_image_dependencies(
        {Core::ImageDependency(FORWARD_PASS,
                               0,
                               {m_settings.samplesMSAA > MSAASamples::x1 ? (uint32_t)2 : 0,
                                m_settings.samplesMSAA > MSAASamples::x1 ? (uint32_t)3 : 1})});
    // Tonemapping
    m_passes[TONEMAPPIN_PASS] = new Core::TonemappingPass(
        m_device, m_window->get_extent(), m_settings.colorFormat, m_settings.softwareAA ? false : true);
    m_passes[TONEMAPPIN_PASS]->set_image_dependencies({Core::ImageDependency(BLOOM_PASS, 0, {0})});

    // FXAA Pass
    m_passes[FXAA_PASS] = new Core::PostProcessPass(m_device,
                                                    m_window->get_extent(),
                                                    m_settings.colorFormat,
                                                    ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl",
                                                    "FXAA",
                                                    m_settings.softwareAA);
    m_passes[FXAA_PASS]->set_image_dependencies({Core::ImageDependency(TONEMAPPIN_PASS, 0, {0})});
    if (!m_settings.softwareAA)
        m_passes[FXAA_PASS]->set_active(false);
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

            connect_pass(m_passes[FORWARD_PASS]);
        }
    }
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END