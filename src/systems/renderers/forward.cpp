#include <engine/systems/renderers/forward.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void ForwardRenderer::on_before_render(Core::Scene* const scene) {
    BaseRenderer::on_before_render(scene);

    if (scene->get_skybox())
    {
        if (scene->get_skybox()->update_enviroment())
            static_cast<Core::ForwardPass*>(m_passes[FORWARD_PASS])
                ->set_envmap_descriptor(
                    Core::ResourceManager::panoramaConverterPass->get_framebuffers()[0].attachmentImages[0],
                    Core::ResourceManager::irradianceComputePass->get_framebuffers()[0].attachmentImages[0]);
    }

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
        m_passes[SHADOW_PASS]->update();

        m_updateShadows = false;

        connect_pass(m_passes[FORWARD_PASS]);
    }
}
void ForwardRenderer::create_passes() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    m_passes.resize(5, nullptr);
    // Shadow Pass
    m_passes[SHADOW_PASS] =
        new Core::VarianceShadowPass(m_device, {SHADOW_RES, SHADOW_RES}, ENGINE_MAX_LIGHTS, m_settings.depthFormat);

    // Forward Pass
    m_passes[FORWARD_PASS] = new Core::ForwardPass(
        m_device, m_window->get_extent(), SRGBA_32F, m_settings.depthFormat, m_settings.samplesMSAA, false);
    m_passes[FORWARD_PASS]->set_image_dependace_table({{iVec2(SHADOW_PASS, 0), {0}}});

    // Bloom Pass
    m_passes[BLOOM_PASS] = new Core::BloomPass(m_device, m_window->get_extent(), Core::ResourceManager::VIGNETTE);
    m_passes[BLOOM_PASS]->set_image_dependace_table({{iVec2(FORWARD_PASS, 0),
                                                      {m_settings.samplesMSAA > MSAASamples::x1 ? (uint32_t)2 : 0,
                                                       m_settings.samplesMSAA > MSAASamples::x1 ? (uint32_t)3 : 1}}});
    // Tonemapping
    m_passes[TONEMAPPIN_PASS] = new Core::PostProcessPass(m_device,
                                                          m_window->get_extent(),
                                                          m_settings.colorFormat,
                                                          Core::ResourceManager::VIGNETTE,
                                                          ENGINE_RESOURCES_PATH "shaders/misc/tonemapping.glsl",
                                                          "TONEMAPPING",
                                                          m_settings.softwareAA ? false : true);
    m_passes[TONEMAPPIN_PASS]->set_image_dependace_table({{iVec2(BLOOM_PASS, 0), {0}}});

    // FXAA Pass
    m_passes[FXAA_PASS] = new Core::PostProcessPass(m_device,
                                                    m_window->get_extent(),
                                                    m_settings.colorFormat,
                                                    Core::ResourceManager::VIGNETTE,
                                                    ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl",
                                                    "FXAA",
                                                    m_settings.softwareAA);
    m_passes[FXAA_PASS]->set_image_dependace_table({{iVec2(TONEMAPPIN_PASS, 0), {0}}});
    if (!m_settings.softwareAA)
        m_passes[FXAA_PASS]->set_active(false);
}

} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END