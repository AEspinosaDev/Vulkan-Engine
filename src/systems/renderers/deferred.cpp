#include <engine/systems/renderers/deferred.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer::on_before_render(Core::Scene* const scene) {
    BaseRenderer::on_before_render(scene);

    if (scene->get_skybox())
    {
        // if (scene->get_skybox()->update_enviroment())
        //     static_cast<Core::ForwardPass*>(m_renderPipeline.renderpasses[FORWARD])
        //         ->set_envmap_descriptor(Core::ResourceManager::panoramaConverterPass->get_attachments()[0].image,
        //                                 Core::ResourceManager::irradianceComputePass->get_attachments()[0].image);
    }

    m_renderPipeline.renderpasses[GEOMETRY_PASS]->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 2);
}

void DeferredRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    BaseRenderer::on_after_render(renderResult, scene);

    if (m_updateShadows)
    {
        m_device.wait();

        const uint32_t SHADOW_RES = (uint32_t)m_shadowQuality;

        m_renderPipeline.renderpasses[SHADOW_PASS]->set_extent({SHADOW_RES, SHADOW_RES});
        m_renderPipeline.renderpasses[SHADOW_PASS]->update();

        m_updateShadows = false;

        connect_renderpass(m_renderPipeline.renderpasses[COMPOSITION_PASS]);
    }
}
void DeferredRenderer::create_renderpasses() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    // Shadow Pass
    Core::VarianceShadowPass* shadowPass = new Core::VarianceShadowPass(
        &m_device, {SHADOW_RES, SHADOW_RES}, totalImagesInFlight, ENGINE_MAX_LIGHTS, m_settings.depthFormat);
    m_renderPipeline.push_renderpass(shadowPass);

    // Geometry Pass
    Core::GeometryPass* geometryPass = new Core::GeometryPass(
        &m_device, m_window->get_extent(), totalImagesInFlight, m_settings.colorFormat, m_settings.depthFormat);
    m_renderPipeline.push_renderpass(geometryPass);

    // Composition Pass
    Core::CompositionPass* compPass = new Core::CompositionPass(&m_device,
                                                                m_window->get_extent(),
                                                                totalImagesInFlight,
                                                                m_settings.colorFormat,
                                                                Core::ResourceManager::VIGNETTE,
                                                                m_softwareAA ? false : true);
    compPass->set_image_dependace_table({{SHADOW_PASS, {0}}, {GEOMETRY_PASS, {0, 1, 2, 3, 4}}});
    m_renderPipeline.push_renderpass(compPass);

    // FXAA Pass
    Core::FXAAPass* fxaaPass = new Core::FXAAPass(&m_device,
                                                  m_window->get_extent(),
                                                  totalImagesInFlight,
                                                  m_settings.colorFormat,
                                                  Core::ResourceManager::VIGNETTE,
                                                  m_softwareAA);
    fxaaPass->set_image_dependace_table({{COMPOSITION_PASS, {0}}});
    m_renderPipeline.push_renderpass(fxaaPass);
    if (!m_softwareAA)
        fxaaPass->set_active(false);
}

} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END