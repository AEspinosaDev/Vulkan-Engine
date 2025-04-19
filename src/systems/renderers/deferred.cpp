#include <engine/systems/renderers/deferred.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer::on_before_render(Core::Scene* const scene) {
    // Update enviroment before
    update_enviroment(scene->get_skybox());
    BaseRenderer::on_before_render(scene);

    // Set clear color
    static_cast<Core::GraphicPass*>(m_passes[GEOMETRY_PASS])
        ->set_attachment_clear_value(
            {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 2);
}

void DeferredRenderer::on_after_render(RenderResult& renderResult, Core::Scene* const scene) {
    BaseRenderer::on_after_render(renderResult, scene);

    if (m_updateShadows)
    {
        m_device->wait();

        const uint32_t SHADOW_RES = (uint32_t)m_shadowQuality;

        m_passes[SHADOW_PASS]->set_extent({SHADOW_RES, SHADOW_RES});
        m_passes[SHADOW_PASS]->resize_attachments();

        m_updateShadows = false;

        connect_pass(m_passes[COMPOSITION_PASS]);
    }
    if (m_updateGI)
    {
        m_device->wait();

        uint32_t voxelRes = get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->get_VXGI_settings().resolution;
        m_passes[VOXELIZATION_PASS]->set_extent({voxelRes, voxelRes});
        m_passes[VOXELIZATION_PASS]->resize_attachments();

        m_updateGI = false;

        connect_pass(m_passes[COMPOSITION_PASS]);
    }
}
void DeferredRenderer::create_passes() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    m_passes.resize(10, nullptr);

    // Sky Generation Pass
    m_passes[SKY_PASS] = new Core::SkyPass(m_device, {1024, 512});

    // Enviroment Pass
    m_passes[ENVIROMENT_PASS] = new Core::EnviromentPass(m_device);
    m_passes[ENVIROMENT_PASS]->set_image_dependencies({Core::ImageDependency(SKY_PASS, 2, {0})});

    // Shadow Pass
    m_passes[SHADOW_PASS] =
        new Core::VarianceShadowPass(m_device, {SHADOW_RES, SHADOW_RES}, ENGINE_MAX_LIGHTS, m_settings.depthFormat);

    // Voxelization Pass
    m_passes[VOXELIZATION_PASS] = new Core::VoxelizationPass(m_device, 256);
    m_passes[VOXELIZATION_PASS]->set_image_dependencies({Core::ImageDependency(SHADOW_PASS, 0, {0})});

    // Geometry Pass
    m_passes[GEOMETRY_PASS] =
        new Core::GeometryPass(m_device, m_window->get_extent(), m_settings.colorFormat, m_settings.depthFormat);
    m_passes[GEOMETRY_PASS]->set_image_dependencies({Core::ImageDependency(ENVIROMENT_PASS, 0, {0}),
                                                     Core::ImageDependency(ENVIROMENT_PASS, 1, {0}),
                                                     Core::ImageDependency(SKY_PASS, 2, {0})});

    // Pre-Composition Pass
    m_passes[PRECOMPOSITION_PASS] = new Core::PreCompositionPass(m_device, m_window->get_extent());
    m_passes[PRECOMPOSITION_PASS]->set_image_dependencies({Core::ImageDependency(GEOMETRY_PASS, 0, {0, 1})});

    // Composition Pass
    m_passes[COMPOSITION_PASS] = new Core::CompositionPass(m_device, m_window->get_extent(), SRGBA_32F, false);
    m_passes[COMPOSITION_PASS]->set_image_dependencies({Core::ImageDependency(SHADOW_PASS, 0, {0}),
                                                        Core::ImageDependency(VOXELIZATION_PASS, {0}),
                                                        Core::ImageDependency(GEOMETRY_PASS, 0, {0, 1, 2, 3, 4}),
                                                        Core::ImageDependency(PRECOMPOSITION_PASS, 1, {0}),
                                                        Core::ImageDependency(ENVIROMENT_PASS, 0, {0}),
                                                        Core::ImageDependency(ENVIROMENT_PASS, 1, {0})});

    // Bloom Pass
    m_passes[BLOOM_PASS] = new Core::BloomPass(m_device, m_window->get_extent());
    m_passes[BLOOM_PASS]->set_image_dependencies({Core::ImageDependency(COMPOSITION_PASS, 0, {0, 1})});

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
void DeferredRenderer::update_enviroment(Core::Skybox* const skybox) {
    if (skybox)
    {
        if (skybox->update_enviroment())
        {

            Core::ResourceManager::upload_skybox_data(m_device, skybox);
            const uint32_t HDRi_EXTENT       = skybox->get_sky_type() == EnviromentType::IMAGE_BASED_ENV
                                                   ? skybox->get_enviroment_map()->get_size().height
                                                   : skybox->get_sky_settings().resolution;
            const uint32_t IRRADIANCE_EXTENT = skybox->get_irradiance_resolution();

            get_pass<Core::EnviromentPass*>(ENVIROMENT_PASS)->set_irradiance_resolution(IRRADIANCE_EXTENT);
            m_passes[ENVIROMENT_PASS]->set_active(true);
            if (skybox->get_sky_type() == EnviromentType::PROCEDURAL_ENV)
                m_passes[SKY_PASS]->set_active(true);

            // ONLY IF: framebuffers needs to be resized
            // -------------------------------------------------------------------
            if (m_passes[ENVIROMENT_PASS]->get_extent().height != HDRi_EXTENT ||
                get_pass<Core::EnviromentPass*>(ENVIROMENT_PASS)->get_irradiance_resolution() != IRRADIANCE_EXTENT)
            {
                m_device->wait();
                if (skybox->get_sky_type() == EnviromentType::PROCEDURAL_ENV)
                {
                    m_passes[SKY_PASS]->set_extent({HDRi_EXTENT * 2, HDRi_EXTENT});
                    m_passes[SKY_PASS]->resize_attachments();
                    connect_pass(m_passes[ENVIROMENT_PASS]);
                }

                m_passes[ENVIROMENT_PASS]->set_extent({HDRi_EXTENT, HDRi_EXTENT});
                m_passes[ENVIROMENT_PASS]->resize_attachments();

                connect_pass(m_passes[GEOMETRY_PASS]);
                connect_pass(m_passes[COMPOSITION_PASS]);
            }
        }
    }
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END