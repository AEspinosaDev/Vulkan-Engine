#include <engine/systems/renderers/forward.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems
{

void ForwardRenderer::on_before_render(Core::Scene *const scene)
{
    BaseRenderer::on_before_render(scene);

    if (scene->get_skybox())
        if (scene->get_skybox()->update_enviroment())
            static_cast<Core::ForwardPass *>(m_renderPipeline.renderpasses[FORWARD])
                ->set_envmap_descriptor(m_renderPipeline.panoramaConverterPass->get_attachments()[0].image,
                                        m_renderPipeline.irradianceComputePass->get_attachments()[0].image);

    m_renderPipeline.renderpasses[FORWARD]->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a});
}

void ForwardRenderer::on_after_render(VkResult &renderResult, Core::Scene *const scene)
{
    BaseRenderer::on_after_render(renderResult, scene);

    if (m_updateShadows)
        update_shadow_quality();
}
void ForwardRenderer::setup_renderpasses()
{
    const uint32_t SHADOW_RES = (uint32_t)m_settings2.shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    // Shadow Pass
    Core::ShadowPass *shadowPass = new Core::ShadowPass(&m_context, {SHADOW_RES, SHADOW_RES}, totalImagesInFlight,
                                                        VK_MAX_LIGHTS, m_settings.depthFormat);
    m_renderPipeline.push_renderpass(shadowPass);

    // Forward Pass
    Core::ForwardPass *forwardPass =
        new Core::ForwardPass(&m_context, m_window->get_extent(), totalImagesInFlight, m_settings.colorFormat,
                              m_settings.depthFormat, m_settings.samplesMSAA, m_settings2.fxaa ? false : true);
    forwardPass->set_image_dependace_table({{SHADOW, {0}}});
    m_renderPipeline.push_renderpass(forwardPass);

    // FXAA Pass
    Core::FXAAPass *fxaaPass = new Core::FXAAPass(&m_context, m_window->get_extent(), totalImagesInFlight,
                                                  m_settings.colorFormat, m_vignette, m_settings2.fxaa);
    fxaaPass->set_image_dependace_table({{FORWARD, {0}}});
    m_renderPipeline.push_renderpass(fxaaPass);
    if (!m_settings2.fxaa)
        fxaaPass->set_active(false);
}

void ForwardRenderer::update_shadow_quality()
{
    m_context.wait_for_device();

    const uint32_t SHADOW_RES = (uint32_t)m_settings2.shadowQuality;
    m_renderPipeline.renderpasses[SHADOW]->set_extent({SHADOW_RES, SHADOW_RES});
    m_renderPipeline.renderpasses[SHADOW]->update();

    m_updateShadows = false;

    connect_renderpass(m_renderPipeline.renderpasses[FORWARD]);
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END