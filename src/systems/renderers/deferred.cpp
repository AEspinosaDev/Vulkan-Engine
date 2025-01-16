#include <engine/systems/renderers/deferred.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer::on_before_render(Core::Scene* const scene) {
    BaseRenderer::on_before_render(scene);

    // Set Deferred Graphic Settings
    Core::CompositionPass* compPass = static_cast<Core::CompositionPass*>(m_passes[COMPOSITION_PASS]);
    compPass->set_SSR_settings(m_SSR);
    compPass->set_VXGI_settings(m_VXGI);
    if (m_VXGI.updateMode == 0)
        m_passes[VOXELIZATION_PASS]->set_active(m_VXGI.enabled);
   
    compPass->enable_AO(m_SSAO.enabled);
    static_cast<Core::PreCompositionPass*>(m_passes[PRECOMPOSITION_PASS])->set_SSAO_settings(m_SSAO);
    static_cast<Core::BloomPass*>(m_passes[BLOOM_PASS])->set_bloom_strength(m_bloomStrength);

    if (scene->get_skybox())
    {
        if (scene->get_skybox()->update_enviroment())
        {
            static_cast<Core::GeometryPass*>(m_passes[GEOMETRY_PASS])
                ->set_envmap_descriptor(
                    Core::ResourceManager::panoramaConverterPass->get_framebuffers()[0].attachmentImages[0],
                    Core::ResourceManager::irradianceComputePass->get_framebuffers()[0].attachmentImages[0]);
        }
        if (scene->get_skybox()->update_enviroment())
            compPass->set_envmap_descriptor(
                Core::ResourceManager::panoramaConverterPass->get_framebuffers()[0].attachmentImages[0],
                Core::ResourceManager::irradianceComputePass->get_framebuffers()[0].attachmentImages[0]);
    }

    m_passes[GEOMETRY_PASS]->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 2);
}

void DeferredRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    BaseRenderer::on_after_render(renderResult, scene);

     if (m_VXGI.updateMode == 1)
        m_passes[VOXELIZATION_PASS]->set_active(false);

    if (m_updateShadows)
    {
        m_device->wait();

        const uint32_t SHADOW_RES = (uint32_t)m_shadowQuality;

        m_passes[SHADOW_PASS]->set_extent({SHADOW_RES, SHADOW_RES});
        m_passes[SHADOW_PASS]->update();

        m_updateShadows = false;

        connect_pass(m_passes[COMPOSITION_PASS]);
    }
}
void DeferredRenderer::create_passes() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    m_passes.resize(8, nullptr);

    // Shadow Pass
    m_passes[SHADOW_PASS] =
        new Core::VarianceShadowPass(m_device, {SHADOW_RES, SHADOW_RES}, ENGINE_MAX_LIGHTS, m_settings.depthFormat);

    // Voxelization Pass
    m_passes[VOXELIZATION_PASS] = new Core::VoxelizationPass(m_device, m_VXGI.resolution);

    // Geometry Pass
    m_passes[GEOMETRY_PASS] =
        new Core::GeometryPass(m_device, m_window->get_extent(), m_settings.colorFormat, m_settings.depthFormat);

    // Pre-Composition Pass
    m_passes[PRECOMPOSITION_PASS] =
        new Core::PreCompositionPass(m_device, m_window->get_extent(), Core::ResourceManager::VIGNETTE);
    m_passes[PRECOMPOSITION_PASS]->set_image_dependencies({Core::ImageDependency(GEOMETRY_PASS, 0, {0, 1})});

    // Composition Pass
    m_passes[COMPOSITION_PASS] =
        new Core::CompositionPass(m_device, m_window->get_extent(), SRGBA_32F, Core::ResourceManager::VIGNETTE, false);
    m_passes[COMPOSITION_PASS]->set_image_dependencies({Core::ImageDependency(SHADOW_PASS, 0, {0}),
                                                        Core::ImageDependency(VOXELIZATION_PASS, {0}),
                                                        Core::ImageDependency(GEOMETRY_PASS, 0, {0, 1, 2, 3, 4}),
                                                        Core::ImageDependency(PRECOMPOSITION_PASS, 1, {0})});
    // Bloom Pass
    m_passes[BLOOM_PASS] = new Core::BloomPass(m_device, m_window->get_extent(), Core::ResourceManager::VIGNETTE);
    m_passes[BLOOM_PASS]->set_image_dependencies({Core::ImageDependency(COMPOSITION_PASS, 0, {0, 1})});

    // Tonemapping
    m_passes[TONEMAPPIN_PASS] = new Core::PostProcessPass(m_device,
                                                          m_window->get_extent(),
                                                          m_settings.colorFormat,
                                                          Core::ResourceManager::VIGNETTE,
                                                          ENGINE_RESOURCES_PATH "shaders/misc/tonemapping.glsl",
                                                          "TONEMAPPING",
                                                          m_settings.softwareAA ? false : true);
    m_passes[TONEMAPPIN_PASS]->set_image_dependencies({Core::ImageDependency(BLOOM_PASS, 0, {0})});

    // FXAA Pass
    m_passes[FXAA_PASS] = new Core::PostProcessPass(m_device,
                                                    m_window->get_extent(),
                                                    m_settings.colorFormat,
                                                    Core::ResourceManager::VIGNETTE,
                                                    ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl",
                                                    "FXAA",
                                                    m_settings.softwareAA);
    m_passes[FXAA_PASS]->set_image_dependencies({Core::ImageDependency(TONEMAPPIN_PASS, 0, {0})});
    if (!m_settings.softwareAA)
        m_passes[FXAA_PASS]->set_active(false);
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END