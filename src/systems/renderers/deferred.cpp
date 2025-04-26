#include <engine/systems/renderers/deferred.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer::on_before_render(Core::Scene* const scene) {
    // Prepare for inverse Z rendering
    auto cam = scene->get_active_camera();
    if (cam)
    {
        if (!cam->inverse_Z())
            scene->get_active_camera()->inverse_Z(true);
    }
    // Update enviroment before
    update_enviroment(scene->get_skybox());
    BaseRenderer::on_before_render(scene);

    // Set clear color (On albedo buffer)
    m_passes[GEOMETRY_PASS]->set_attachment_clear_value(
        {m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}, 1);
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

    // Create Passes and Attachments pool
    //--------------------------------
    m_passes.resize(11, nullptr);
    m_attachments.resize(17);

    // Arrange connectivity
    //--------------------------------
    Core::PassLinkage<0, 1>  skyPassConfig         = {m_attachments, {}, {0}};
    Core::PassLinkage<1, 2>  enviromentPassConfig  = {m_attachments, {0}, {1, 2}};
    Core::PassLinkage<0, 2>  shadowPassConfig      = {m_attachments, {}, {3, 4}};
    Core::PassLinkage<1, 1>  voxelPassConfig       = {m_attachments, {3}, {5}};
    Core::PassLinkage<3, 5>  geometryPassConfig    = {m_attachments, {1, 2, 0}, {6, 7, 8, 9, 10}};
    Core::PassLinkage<2, 1>  preCompPassConfig     = {m_attachments, {10, 6}, {12}};
    Core::PassLinkage<10, 2> compPassConfig        = {m_attachments, {3, 5, 10, 6, 7, 8, 9, 12, 1, 2}, {13, 14}};
    Core::PassLinkage<2, 1>  bloomPassConfig       = {m_attachments, {13, 14}, {15}};
    Core::PassLinkage<1, 1>  toneMappingPassConfig = {m_attachments, {15}, {16}};
    Core::PassLinkage<1, 0>  FXAAPassConfig        = {m_attachments, {16}, {}};

    // Create passes
    //--------------------------------

    m_passes[SKY_PASS]        = new Core::SkyPass(m_device, skyPassConfig, {1024, 512});
    m_passes[ENVIROMENT_PASS] = new Core::EnviromentPass(m_device, enviromentPassConfig);
    m_passes[SHADOW_PASS]     = new Core::VarianceShadowPass(m_device,
                                                         shadowPassConfig,
                                                             {(uint32_t)m_shadowQuality, (uint32_t)m_shadowQuality},
                                                         ENGINE_MAX_LIGHTS,
                                                         m_settings.depthFormat);

    m_passes[VOXELIZATION_PASS] = new Core::VoxelizationPass(m_device, voxelPassConfig, 256);

    m_passes[GEOMETRY_PASS] = new Core::GeometryPass(
        m_device, geometryPassConfig, m_window->get_extent(), m_settings.colorFormat, m_settings.depthFormat);

    m_passes[PRECOMPOSITION_PASS] = new Core::PreCompositionPass(m_device, preCompPassConfig, m_window->get_extent());

    m_passes[COMPOSITION_PASS] = new Core::CompositionPass(m_device, compPassConfig, m_window->get_extent(), SRGBA_16F);

    m_passes[BLOOM_PASS] = new Core::BloomPass(m_device, bloomPassConfig, m_window->get_extent());

    m_passes[TONEMAPPIN_PASS] = new Core::TonemappingPass(m_device,
                                                          toneMappingPassConfig,
                                                          m_window->get_extent(),
                                                          m_settings.colorFormat,
                                                          m_settings.softwareAA == SoftwareAA::NONE ? true : false);

    m_passes[FXAA_PASS] = new Core::PostProcessPass<1, 0>(m_device,
                                                          FXAAPassConfig,
                                                          m_window->get_extent(),
                                                          m_settings.colorFormat,
                                                          ENGINE_RESOURCES_PATH "shaders/aa/fxaa.glsl",
                                                          "FXAA",
                                                          m_settings.softwareAA == SoftwareAA::FXAA ? true : false);

    m_passes[GUI_PASS] = new Core::GUIPass(m_device, m_window->get_extent());

    //--------------------------------

    if (m_settings.softwareAA != SoftwareAA::FXAA)
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
