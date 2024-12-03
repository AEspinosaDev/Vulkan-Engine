#include <engine/systems/renderers/deferred.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer::on_before_render(Core::Scene* const scene) {
    BaseRenderer::on_before_render(scene);

    if (scene->get_skybox())
    {
        if (scene->get_skybox()->update_enviroment())
            static_cast<Core::GeometryPass*>(m_passes[GEOMETRY_PASS])
                ->set_envmap_descriptor(Core::ResourceManager::panoramaConverterPass->get_attachments()[0].image,
                                        Core::ResourceManager::irradianceComputePass->get_attachments()[0].image);
        if (scene->get_skybox()->update_enviroment())
            static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS])
                ->set_envmap_descriptor(Core::ResourceManager::panoramaConverterPass->get_attachments()[0].image,
                                        Core::ResourceManager::irradianceComputePass->get_attachments()[0].image);
    }

    m_passes[GEOMETRY_PASS]->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 2);
}

void DeferredRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    BaseRenderer::on_after_render(renderResult, scene);

    if (m_updateShadows)
    {
        m_device.wait();

        const uint32_t SHADOW_RES = (uint32_t)m_shadowQuality;

        m_passes[SHADOW_PASS]->set_extent({SHADOW_RES, SHADOW_RES});
        m_passes[SHADOW_PASS]->update();

        m_updateShadows = false;

        connect_renderpass(m_passes[COMPOSITION_PASS]);
    }
}
void DeferredRenderer::create_renderpasses() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    m_passes.resize(5, nullptr);
    // Shadow Pass
    m_passes[SHADOW_PASS] = new Core::VarianceShadowPass(
        &m_device, {SHADOW_RES, SHADOW_RES}, totalImagesInFlight, ENGINE_MAX_LIGHTS, m_settings.depthFormat);

    // Geometry Pass
    m_passes[GEOMETRY_PASS] = new Core::GeometryPass(
        &m_device, m_window->get_extent(), totalImagesInFlight, m_settings.colorFormat, m_settings.depthFormat);

    // Composition Pass
    m_passes[COMPOSITION_PASS] = new Core::CompositionPass(&m_device,
                                                           m_window->get_extent(),
                                                           totalImagesInFlight,
                                                           m_settings.colorFormat,
                                                           Core::ResourceManager::VIGNETTE,
                                                           false);
    m_passes[COMPOSITION_PASS]->set_image_dependace_table({{SHADOW_PASS, {0}}, {GEOMETRY_PASS, {0, 1, 2, 3, 4}}});
    // Tonemapping
    m_passes[TONEMAPPIN_PASS] = new Core::PostProcessPass(&m_device,
                                                          m_window->get_extent(),
                                                          totalImagesInFlight,
                                                          m_settings.colorFormat,
                                                          Core::ResourceManager::VIGNETTE,
                                                          ENGINE_RESOURCES_PATH "shaders/misc/tonemapping.glsl",
                                                          "TONEMAPPING",
                                                          m_settings.softwareAA ? false : true);
    m_passes[TONEMAPPIN_PASS]->set_image_dependace_table({{COMPOSITION_PASS, {0}}});
    // FXAA Pass
    m_passes[FXAA_PASS] = new Core::PostProcessPass(&m_device,
                                                    m_window->get_extent(),
                                                    totalImagesInFlight,
                                                    m_settings.colorFormat,
                                                    Core::ResourceManager::VIGNETTE,
                                                    ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl",
                                                    "FXAA",
                                                    m_settings.softwareAA);
    m_passes[FXAA_PASS]->set_image_dependace_table({{TONEMAPPIN_PASS, {0}}});
    if (!m_settings.softwareAA)
        m_passes[FXAA_PASS]->set_active(false);
}

} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END