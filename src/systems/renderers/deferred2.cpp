#include <engine/systems/renderers/deferred2.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Systems {

void DeferredRenderer2::on_before_render( Core::Scene* const scene ) {
    // Prepare for inverse Z rendering
    auto cam = scene->get_active_camera();
    if ( cam )
    {
        if ( !cam->inverse_Z() )
            cam->inverse_Z( true );
    }
}

void DeferredRenderer2::init_resources() {

    // Setup frames
    m_frames.resize( static_cast<uint32_t>( m_settings.bufferingType ) );
    for ( size_t i = 0; i < m_frames.size(); i++ )
    {
        // Create Frame
        m_frames[i].init( m_device, i );
        // Register main UBOs
        m_frames[i].register_UBO<Render::CameraUniforms, Render::SceneUniforms>( "globalUBO" );
        m_frames[i].register_UBO_array<Render::ObjectUniforms, Render::MaterialUniforms>( "objectUBO", ENGINE_MAX_OBJECTS );
    }

    // m_renderResources.init_shared_resources( m_device );

    // // Create basic texture resources
    // m_renderResources.sharedTextures.resize( 2, nullptr );

    // Core::TextureLDR* samplerText = new Core::TextureLDR();
    // Tools::Loaders::load_PNG( samplerText, GET_RESOURCE_PATH( "textures/blueNoise.png" ), TEXTURE_FORMAT_UNORM );
    // samplerText->set_use_mipmaps( false );
    // m_renderResources.upload_texture_data( m_device, samplerText );
    // m_renderResources.sharedTextures[0] = samplerText;
}
void DeferredRenderer2::register_shaders() {

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> shadowsBindings = {
        { "u_camera", { .set = 0, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_scene", { .set = 0, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_object", { .set = 1, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
        { "u_material", { .set = 1, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "ShadowProgram", GET_RESOURCE_PATH( "shaders/shadows/vsm_geom.glsl" ), shadowsBindings );

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> geomBindings = {
        { "u_camera", { .set = 0, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_scene", { .set = 0, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_object", { .set = 1, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
        { "u_material", { .set = 1, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
        { "u_textures", { .set = 2, .binding = 0, .type = UniformType::ImageSampler, .bindless = true, .source = BindingSource::Manual } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "GeometryProgram", GET_RESOURCE_PATH( "shaders/deferred/geometry.glsl" ), geomBindings );

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> skyGeomBindings = {
        { "u_camera", { .set = 0, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_scene", { .set = 0, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "SkyboxGeometryProgram", GET_RESOURCE_PATH( "shaders/deferred/skybox.glsl" ), skyGeomBindings );

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> compBindings = {
        { "u_camera", { .set = 0, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_scene", { .set = 0, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_shadows", { .set = 0, .binding = 2, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "shadows" } },
        // { "u_env", { .set = 0, .binding = 3, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "enviroment" } },
        // { "u_accel", { .set = 0, .binding = 4, .type = UniformType::AccelerationStructure, .source = BindingSource::Shared, .resourceName = "tlas" } },
        // { "u_noise", { .set = 0, .binding = 5, .type = UniformType::ImageSampler, .source = BindingSource::Shared, .resourceName = "noiseTex" } },
        // { "u_voxel", { .set = 0, .binding = 6, .type = UniformType::ImageSampler, .source = BindingSource::Shared, .resourceName = "voxelTex" } },

        { "u_depth", { .set = 1, .binding = 0, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "depth" } },
        { "u_normal", { .set = 1, .binding = 1, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "normals" } },
        { "u_albedo", { .set = 1, .binding = 2, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "albedo" } },
        { "u_material", { .set = 1, .binding = 3, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "material" } },
        { "u_emissison", { .set = 1, .binding = 4, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "emission" } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "LightingProgram", GET_RESOURCE_PATH( "shaders/deferred/composition.glsl" ), compBindings );

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> toneBindings = {
        { "u_input", { .set = 0, .binding = 0, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "lighting" } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "TonemapProgram", GET_RESOURCE_PATH( "shaders/misc/tonemapping.glsl" ), toneBindings );
}

void DeferredRenderer2::configure_passes() {

    // Main configs
    const Extent2D        DISPLAY_EXTENT = !m_headless ? m_window->get_extent() : m_headlessExtent;
    const ColorFormatType HDR_FORMAT     = m_settings.highDynamicPrecission == FloatPrecission::F16 ? SRGBA_16F : SRGBA_32F;
    const ColorFormatType DEPTH_FORMAT   = m_settings.depthPrecission == FloatPrecission::F16 ? DEPTH_16F : DEPTH_32F;
    const Extent2D        SHADOW_RES     = { (uint32_t)m_settings.shadowQuality, (uint32_t)m_settings.shadowQuality };
    const Extent2D        SKY_RES        = { 1024, 512 };

    // SHADOW PASS
    m_graph.add_pass( "ShadowPass", { "ShadowProgram" }, [&]( Render::RenderGraphBuilder& builder ) {
            builder.create_target("shadows", Render::TargetInfo{
                .extent = SHADOW_RES,
                .format = SRG_32F,
                .usage =  IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                .finalLayout = LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .clearValue = {.depthStencil.depth = 1.0f},
                .load = false,
                .store = true,
                .layers = ENGINE_MAX_LIGHTS
            }); 
            builder.create_depth_target("shadows_depth", SHADOW_RES, m_settings.depthPrecission, ENGINE_MAX_LIGHTS); }, [&]( const Render::RenderView& view, Render::Frame& frame, const Render::Resources& shared, const Render::RenderPassOutputs& outputs ) {
                auto& cmd = frame.get_command_buffer();
                
                cmd.begin_renderpass(outputs.renderPass, outputs.fbos[0]);
                cmd.set_viewport( SHADOW_RES );
                
                cmd.set_depth_bias_enable( true );
                float depthBiasConstant = 0.0;
                float depthBiasSlope    = 0.0f;
                cmd.set_depth_bias( depthBiasConstant, 0.0f, depthBiasSlope );
                
                for ( const auto& drawCallIdx : view.shadowDrawCalls )
                {
                    auto drawCall = view.drawCalls[drawCallIdx];
                    auto program = m_graph.get_shader_program( "ShadowProgram" );

                    cmd.set_depth_test_enable( drawCall.params.depthTest );
                    cmd.set_depth_write_enable( drawCall.params.depthWrites );
                    cmd.set_cull_mode( drawCall.params.culling );

                    program->bind(frame);
                    program->bind_uniform_set(0,frame, {0,0});
                    program->bind_uniform_set(1,frame,  { drawCall.bufferOffset, drawCall.bufferOffset });
                    
                    // DRAW
                    cmd.draw_geometry( drawCall.vertexArrays );
                }
                
                cmd.end_renderpass( outputs.renderPass, outputs.fbos[0] ); } );

    // GEOMETRY PASS
    m_graph.add_pass( "GeometryPass", { "GeometryProgram" }, [&]( Render::RenderGraphBuilder& builder ) {
        // Normals + Depth
        builder.create_target( "normals", Render::TargetInfo { .extent = DISPLAY_EXTENT, .format = HDR_FORMAT, .usage = IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST, .finalLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL, .clearValue = { .depthStencil.depth = 1.0f } } );
        // Albedo + Opacity
        builder.create_target( "albedo", Render::TargetInfo {
                                             .extent      = DISPLAY_EXTENT,
                                             .format      = RGBA_8U,
                                             .usage       = IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                             .finalLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             .clearValue  ={.color = {{0, 0, 0, 1}}},
                                         } );
        // Material + ID
        builder.create_target( "material", Render::TargetInfo {
                                               .extent      = DISPLAY_EXTENT,
                                               .format      = RGBA_8U,
                                               .usage       = IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                               .finalLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               .clearValue  = {.color = {{0, 0, 0, 1}}},
                                           } );
        // Velocity + Emissive strength
        builder.create_target( "emissive", Render::TargetInfo {
                                               .extent      = DISPLAY_EXTENT,
                                               .format      = HDR_FORMAT,
                                               .usage       = IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                               .finalLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               .clearValue  = {.color = {{0, 0, 0, 1}}},
                                           } );
        builder.create_depth_target( "depth", DISPLAY_EXTENT, m_settings.depthPrecission ); }, [&]( const Render::RenderView& view, Render::Frame& frame, const Render::Resources& shared, const Render::RenderPassOutputs& outputs ) {
        auto     program = m_graph.get_shader_program( "GeometryProgram" );
        uint32_t dcIdx   = 0;
        for ( const auto& drawCallIdx : view.opaqueDrawCalls )
        {
            auto drawCall = view.drawCalls[drawCallIdx];

            for ( auto& texture : drawCall.textureBatch )
            {
                if ( !texture.img->image->empty )
                    program->attach( "u_textures",  Render::UniformResource{ *texture.img }, uint32_t(texture.binding + 6 * dcIdx), frame );

                dcIdx++;
            }
        }

            auto& cmd = frame.get_command_buffer();
            cmd.begin_renderpass( outputs.renderPass, outputs.fbos[0] );
            cmd.set_viewport( DISPLAY_EXTENT );

            // Skybox
            if ( view.enviromentDrawCall > 0 )
            {
                cmd.set_depth_test_enable( false );
                cmd.set_depth_write_enable( false );
                cmd.set_cull_mode( CullingMode::NO_CULLING );

                auto program = m_graph.get_shader_program( "SkyboxGeometryProgram" );
                program->bind( frame );
                program->bind_uniform_set( 0, frame, { 0, 0 } );

                auto drawCall = view.drawCalls[view.enviromentDrawCall];
                cmd.draw_geometry( drawCall.vertexArrays );
            }

            program = m_graph.get_shader_program( "GeometryProgram" );
            program->bind( frame );
            program->bind_uniform_set( 2, frame );

            for ( const auto& drawCallIdx : view.opaqueDrawCalls )
            {
                auto drawCall = view.drawCalls[drawCallIdx];
                if ( drawCall.culled )
                    continue;

                cmd.set_depth_test_enable( drawCall.params.depthTest );
                cmd.set_depth_write_enable( drawCall.params.depthWrites );
                cmd.set_cull_mode( drawCall.params.culling );

                program->bind_uniform_set( 0, frame, { 0, 0 } );
                program->bind_uniform_set( 1, frame, { drawCall.bufferOffset, drawCall.bufferOffset } );

                cmd.draw_geometry( drawCall.vertexArrays );
            }

            cmd.end_renderpass( outputs.renderPass, outputs.fbos[0] ); } );

    // LIGHTING PASS
    m_graph.add_pass( "LightingPass", { "LightingProgram" }, [&]( Render::RenderGraphBuilder& builder ) {
        builder.read( "normals", {} );
        builder.read( "albedo", {} );
        builder.read( "material", {} );
        builder.read( "emissive", {} );
        builder.read( "shadows", {} );

        builder.create_target( "lighting", {
                                               .extent      = DISPLAY_EXTENT,
                                               .format      = HDR_FORMAT,
                                               .usage       = IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                               .finalLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               .clearValue  = { .color = { { 0, 0, 0, 1 } } },
                                           } ); }, [&]( const Render::RenderView& view, Render::Frame& frame, const Render::Resources& shared, const Render::RenderPassOutputs& outputs ) {
        auto program = m_graph.get_shader_program( "LightingProgram" );

        auto& cmd = frame.get_command_buffer();

        cmd.begin_renderpass( outputs.renderPass, outputs.fbos[0] );
        cmd.set_viewport( DISPLAY_EXTENT );

        program->bind( frame );

        program->bind_uniform_set( 0, frame, { 0, 0 } );
        program->bind_uniform_set( 1, frame );

        cmd.draw_geometry( *get_VAO( shared.vignette ) );

        cmd.end_renderpass( outputs.renderPass, outputs.fbos[0] ); } );

    // TONEMAP PASS
    m_graph.add_pass( "TonemapPass", { "TonemapProgram" }, [&]( Render::RenderGraphBuilder& builder ) {
        builder.read( "normals", {} );
        builder.read( "albedo", {} );
        builder.read( "material", {} );
        builder.read( "emissive", {} );
        builder.read( "shadows", {} );

        builder.create_target( "lighting", {
                                               .extent      = DISPLAY_EXTENT,
                                               .format      = HDR_FORMAT,
                                               .usage       = IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                               .finalLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               .clearValue  = { .color = { { 0, 0, 0, 1 } } },
                                           } ); }, [&]( const Render::RenderView& view, Render::Frame& frame, const Render::Resources& shared, const Render::RenderPassOutputs& outputs ) {
        auto program = m_graph.get_shader_program( "TonemapProgram" );

        auto& cmd = frame.get_command_buffer();

        cmd.begin_renderpass( outputs.renderPass, outputs.fbos[0] );
        cmd.set_viewport( DISPLAY_EXTENT );

        program->bind( frame );

        program->bind_uniform_set( 0, frame );

        cmd.draw_geometry( *get_VAO( shared.vignette ) );

        cmd.end_renderpass( outputs.renderPass, outputs.fbos[0] ); } );
}

} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END
