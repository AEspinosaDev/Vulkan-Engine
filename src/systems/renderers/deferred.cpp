#include <engine/systems/renderers/deferred.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer::on_before_render( Core::Scene* const scene ) {
    // Prepare for inverse Z rendering
    auto cam = scene->get_active_camera();
    if ( cam )
    {
        if ( !cam->inverse_Z() )
            cam->inverse_Z( true );
    }
    // Update enviroment before
    update_enviroment( scene->get_skybox() );
    BaseRenderer::on_before_render( scene );

    // Set clear color (On albedo buffer)
    m_passes[GEOMETRY_PASS]->set_attachment_clear_value(
        { m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a }, 1 );
}

void DeferredRenderer::on_after_render( RenderResult& renderResult, Core::Scene* const scene ) {
    BaseRenderer::on_after_render( renderResult, scene );

    if ( m_updateShadows )
    {
        m_device->wait_idle();

        const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowQuality;

        m_passes[SHADOW_PASS]->set_extent( { SHADOW_RES, SHADOW_RES } );
        m_passes[SHADOW_PASS]->resize_attachments();

        m_updateShadows = false;

        m_passes[COMPOSITION_PASS]->link_input_attachments();
    }
    if ( m_updateGI )
    {
        m_device->wait_idle();

        uint32_t voxelRes = get_pass<Render::CompositionPass>( COMPOSITION_PASS )->get_VXGI_settings().resolution;
        m_passes[VOXELIZATION_PASS]->set_extent( { voxelRes, voxelRes } );
        m_passes[VOXELIZATION_PASS]->resize_attachments();

        m_updateGI = false;

        m_passes[COMPOSITION_PASS]->link_input_attachments();
    }
}
void DeferredRenderer::create_passes() {

    // Create Passes and Attachments pool
    //--------------------------------
    m_passes.resize( 11, nullptr );
    m_attachments.resize( 18 );

    // Main configs
    const Extent2D        DISPLAY_EXTENT = !m_headless ? m_window->get_extent() : m_headlessExtent;
    const ColorFormatType HDR_FORMAT     = m_settings.highDynamicPrecission == FloatPrecission::F16 ? SRGBA_16F : SRGBA_32F;
    const ColorFormatType DEPTH_FORMAT   = m_settings.depthPrecission == FloatPrecission::F16 ? DEPTH_16F : DEPTH_32F;
    const Extent2D        SHADOW_RES     = { (uint32_t)m_settings.shadowQuality, (uint32_t)m_settings.shadowQuality };
    const Extent2D        SKY_RES        = { 1024, 512 };

    // Arrange connectivity
    //--------------------------------
    Render::PassLinkage<0, 1>  skyPassConfig         = { m_attachments, {}, { 0 } };
    Render::PassLinkage<1, 2>  enviromentPassConfig  = { m_attachments, { 0 }, { 1, 2 } };
    Render::PassLinkage<0, 2>  shadowPassConfig      = { m_attachments, {}, { 3, 4 } };
    Render::PassLinkage<1, 1>  voxelPassConfig       = { m_attachments, { 3 }, { 5 } };
    Render::PassLinkage<3, 5>  geometryPassConfig    = { m_attachments, { 1, 2, 0 }, { 6, 7, 8, 9, 10 } };
    Render::PassLinkage<2, 1>  preCompPassConfig     = { m_attachments, { 10, 6 }, { 12 } };
    Render::PassLinkage<10, 2> compPassConfig        = { m_attachments, { 3, 5, 10, 6, 7, 8, 9, 12, 1, 2 }, { 13, 14 } };
    Render::PassLinkage<2, 1>  bloomPassConfig       = { m_attachments, { 13, 14 }, { 15 } };
    Render::PassLinkage<2, 1>  TAAPassConfig         = { m_attachments, { 15, 9 }, { 16 } };
    Render::PassLinkage<1, 1>  FXAAPassConfig        = { m_attachments, { 15 }, { 16 } };
    Render::PassLinkage<1, 1>  toneMappingPassConfig = { m_attachments, { 16 }, { 17 } };

    // Create passes
    //--------------------------------

    m_passes[SKY_PASS]            = std::make_shared<Render::SkyPass>( m_device, m_shared, skyPassConfig, SKY_RES );
    m_passes[ENVIROMENT_PASS]     = std::make_shared<Render::EnviromentPass>( m_device, m_shared, enviromentPassConfig );
    m_passes[SHADOW_PASS]         = std::make_shared<Render::VarianceShadowPass>( m_device, m_shared, shadowPassConfig, SHADOW_RES, ENGINE_MAX_LIGHTS, DEPTH_FORMAT );
    m_passes[VOXELIZATION_PASS]   = std::make_shared<Render::VoxelizationPass>( m_device, m_shared, voxelPassConfig, 256 );
    m_passes[GEOMETRY_PASS]       = std::make_shared<Render::GeometryPass>( m_device, m_shared, geometryPassConfig, DISPLAY_EXTENT, HDR_FORMAT, DEPTH_FORMAT );
    m_passes[PRECOMPOSITION_PASS] = std::make_shared<Render::PreCompositionPass>( m_device, m_shared, preCompPassConfig, DISPLAY_EXTENT );
    m_passes[COMPOSITION_PASS]    = std::make_shared<Render::CompositionPass>( m_device, m_shared, compPassConfig, DISPLAY_EXTENT, HDR_FORMAT );
    m_passes[BLOOM_PASS]          = std::make_shared<Render::BloomPass>( m_device, m_shared, bloomPassConfig, DISPLAY_EXTENT, HDR_FORMAT );

    if ( m_settings.softwareAA == SoftwareAA::FXAA || m_settings.softwareAA == SoftwareAA::NONE )
        m_passes[AA_PASS] = std::make_shared<Render::PostProcessPass<1, 1>>(
            m_device, m_shared, FXAAPassConfig, DISPLAY_EXTENT, HDR_FORMAT, GET_RESOURCE_PATH( "shaders/aa/fxaa.glsl" ), "FXAA", false );
    if ( m_settings.softwareAA == SoftwareAA::TAA )
        m_passes[AA_PASS] = std::make_shared<Render::TAAPass>( m_device, m_shared, TAAPassConfig, DISPLAY_EXTENT, HDR_FORMAT, false );

    m_passes[TONEMAPPIN_PASS] =
        std::make_shared<Render::TonemappingPass>( m_device, m_shared, toneMappingPassConfig, DISPLAY_EXTENT, m_settings.displayColorFormat, !m_headless ? true : false );

    m_passes[GUI_PASS] = std::make_shared<Render::GUIPass>( m_device, m_shared, DISPLAY_EXTENT );

    //--------------------------------

    if ( m_settings.softwareAA == SoftwareAA::NONE )
        m_passes[AA_PASS]->set_active( false );
    if ( m_headless )
        m_passes[GUI_PASS]->set_active( false );
}
void DeferredRenderer::update_enviroment( Core::Skybox* const skybox ) {
    if ( skybox )
    {
        if ( skybox->update_enviroment() )
        {

            m_gpuScene.build_skybox_data( m_device, skybox );
            const uint32_t HDRi_EXTENT       = skybox->get_sky_type() == EnviromentType::IMAGE_BASED_ENV ? skybox->get_enviroment_map()->get_size().height
                                                                                                         : skybox->get_sky_settings().resolution;
            const uint32_t IRRADIANCE_EXTENT = skybox->get_irradiance_resolution();

            get_pass<Render::EnviromentPass>( ENVIROMENT_PASS )->set_irradiance_resolution( IRRADIANCE_EXTENT );
            m_passes[ENVIROMENT_PASS]->set_active( true );
            if ( skybox->get_sky_type() == EnviromentType::PROCEDURAL_ENV )
                m_passes[SKY_PASS]->set_active( true );

            // ONLY IF: framebuffers needs to be resized
            // -------------------------------------------------------------------
            if ( m_passes[ENVIROMENT_PASS]->get_extent().height != HDRi_EXTENT ||
                 get_pass<Render::EnviromentPass>( ENVIROMENT_PASS )->get_irradiance_resolution() != IRRADIANCE_EXTENT )
            {
                m_device->wait_idle();
                if ( skybox->get_sky_type() == EnviromentType::PROCEDURAL_ENV )
                {
                    m_passes[SKY_PASS]->set_extent( { HDRi_EXTENT * 2, HDRi_EXTENT } );
                    m_passes[SKY_PASS]->resize_attachments();
                    m_passes[ENVIROMENT_PASS]->link_input_attachments();
                }

                m_passes[ENVIROMENT_PASS]->set_extent( { HDRi_EXTENT, HDRi_EXTENT } );
                m_passes[ENVIROMENT_PASS]->resize_attachments();

                m_passes[GEOMETRY_PASS]->link_input_attachments();
                m_passes[COMPOSITION_PASS]->link_input_attachments();
            }
        }
    }
}
} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END
