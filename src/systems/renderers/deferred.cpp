#include <engine/systems/renderers/deferred.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer::on_before_render(Core::Scene* const scene) {
    // Update enviroment before
    update_enviroment(scene->get_skybox());
    BaseRenderer::on_before_render(scene);

    // Set clear color
    m_passes[GEOMETRY_PASS]->set_attachment_clear_value(
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

        m_passes[COMPOSITION_PASS]->link_input_attachments();
    }
    if (m_updateGI)
    {
        m_device->wait();

        uint32_t voxelRes = get_pass<Core::CompositionPass*>(COMPOSITION_PASS)->get_VXGI_settings().resolution;
        m_passes[VOXELIZATION_PASS]->set_extent({voxelRes, voxelRes});
        m_passes[VOXELIZATION_PASS]->resize_attachments();

        m_updateGI = false;

        m_passes[COMPOSITION_PASS]->link_input_attachments();
    }
}
void DeferredRenderer::create_passes() {
    const uint32_t SHADOW_RES          = (uint32_t)m_shadowQuality;
    const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

    //Create Passes and Attachments pool
    //--------------------------------
    m_passes.resize(10, nullptr);
    m_attachments.resize(17, {});
    
    // Arrange connectivity
    //--------------------------------
    Core::BasePass::attachmentPool = m_attachments;

    Core::PassConfig<0, 1>  skyPassConfig         = {{}, {0}};
    Core::PassConfig<1, 2>  enviromentPassConfig  = {{0}, {1, 2}};
    Core::PassConfig<0, 2>  shadowPassConfig      = {{}, {3, 4}};
    Core::PassConfig<1, 1>  voxelPassConfig       = {{3}, {5}};
    Core::PassConfig<3, 6>  geometryPassConfig    = {{1, 2, 0}, {6, 7, 8, 9, 10, 11}};
    Core::PassConfig<2, 1>  preCompPassConfig     = {{6, 7}, {12}};
    Core::PassConfig<10, 2> compPassConfig        = {{3, 5, 6, 7, 8, 9, 10, 12, 1, 2}, {13, 14}};
    Core::PassConfig<2, 1>  bloomPassConfig       = {{13, 14}, {15}};
    Core::PassConfig<1, 1>  toneMappingPassConfig = {{15}, {16}};
    Core::PassConfig<1, 1>  FXAAPassConfig        = {{16}, {}};
    toneMappingPassConfig.isDefault               = m_settings.softwareAA ? false : true;
    FXAAPassConfig.isDefault                      = m_settings.softwareAA;

    // Create passes
    //--------------------------------
    // m_passes[SKY_PASS]        = Systems::PassFactory::instance().create("MyCustomPass", m_device, &skyPassConfig).get();

    m_passes[SKY_PASS]        = new Core::SkyPass(m_device, skyPassConfig, {1024, 512});
    m_passes[ENVIROMENT_PASS] = new Core::EnviromentPass(m_device, enviromentPassConfig);
    m_passes[SHADOW_PASS]     = new Core::VarianceShadowPass(
        m_device, shadowPassConfig, {SHADOW_RES, SHADOW_RES}, ENGINE_MAX_LIGHTS, m_settings.depthFormat);

    m_passes[VOXELIZATION_PASS] = new Core::VoxelizationPass(m_device, voxelPassConfig, 256);

    m_passes[GEOMETRY_PASS] = new Core::GeometryPass(
        m_device, geometryPassConfig, m_window->get_extent(), m_settings.colorFormat, m_settings.depthFormat);

    m_passes[PRECOMPOSITION_PASS] = new Core::PreCompositionPass(m_device, preCompPassConfig, m_window->get_extent());

    m_passes[COMPOSITION_PASS] =
        new Core::CompositionPass(m_device, compPassConfig, m_window->get_extent(), SRGBA_32F, false);

    m_passes[BLOOM_PASS] = new Core::BloomPass(m_device, bloomPassConfig, m_window->get_extent());

    m_passes[TONEMAPPIN_PASS] =
        new Core::TonemappingPass(m_device, toneMappingPassConfig, m_window->get_extent(), m_settings.colorFormat);

    m_passes[FXAA_PASS] = new Core::PostProcessPass<1, 1>(m_device,
                                                          FXAAPassConfig,
                                                          m_window->get_extent(),
                                                          m_settings.colorFormat,
                                                          ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl",
                                                          "FXAA");
    //--------------------------------

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
                    m_passes[ENVIROMENT_PASS]->link_input_attachments();
                }

                m_passes[ENVIROMENT_PASS]->set_extent({HDRi_EXTENT, HDRi_EXTENT});
                m_passes[ENVIROMENT_PASS]->resize_attachments();

                m_passes[GEOMETRY_PASS]->link_input_attachments();
                m_passes[COMPOSITION_PASS]->link_input_attachments();
            }
        }
    }
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END
