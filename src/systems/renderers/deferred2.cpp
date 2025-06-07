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

    // Core::TextureHDR* brdfText = new Core::TextureHDR();
    // Tools::Loaders::load_HDRi( brdfText, GET_RESOURCE_PATH( "textures/cookTorranceBRDF.png" ) );
    // brdfText->set_adress_mode( ADDRESS_MODE_CLAMP_TO_BORDER );
    // brdfText->set_use_mipmaps( false );
    // m_renderResources.upload_texture_data( m_device, brdfText );
    // m_renderResources.sharedTextures[1] = brdfText;
}
void DeferredRenderer2::register_shaders() {

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> shadowsBindings = {
        { "u_camera", { .set = 0, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_scene", { .set = 0, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_object", { .set = 1, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
        { "u_material", { .set = 1, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "shadows", GET_RESOURCE_PATH( "shaders/shadows/vsm_geom.glsl" ), shadowsBindings );

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> geomBindings = {
        { "u_camera", { .set = 0, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_scene", { .set = 0, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_object", { .set = 1, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
        { "u_material", { .set = 1, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "objectUBO" } },
        { "u_textures", { .set = 2, .binding = 0, .type = UniformType::ImageSampler, .bindless = true, .source = BindingSource::Manual } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "geometry", GET_RESOURCE_PATH( "shaders/deferred/geometry.glsl" ), geomBindings );

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> compBindings = {
        { "u_camera", { .set = 0, .binding = 0, .type = UniformType::DynamicBuffer, .dynamicOffset = 0, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_scene", { .set = 0, .binding = 1, .type = UniformType::DynamicBuffer, .dynamicOffset = 1, .source = BindingSource::Frame, .resourceName = "globalUBO" } },
        { "u_shadows", { .set = 0, .binding = 2, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "shadows" } },
        { "u_env", { .set = 0, .binding = 3, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "enviroment" } },
        { "u_accel", { .set = 0, .binding = 4, .type = UniformType::AccelerationStructure, .source = BindingSource::Shared, .resourceName = "tlas" } },
        { "u_noise", { .set = 0, .binding = 5, .type = UniformType::ImageSampler, .source = BindingSource::Shared, .resourceName = "noiseTex" } },
        { "u_voxel", { .set = 0, .binding = 6, .type = UniformType::ImageSampler, .source = BindingSource::Shared, .resourceName = "voxelTex" } },

        { "u_depth", { .set = 1, .binding = 0, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "depth" } },
        { "u_normal", { .set = 1, .binding = 1, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "normals" } },
        { "u_albedo", { .set = 1, .binding = 2, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "albedo" } },
        { "u_material", { .set = 1, .binding = 3, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "material" } },
        { "u_emisison", { .set = 1, .binding = 4, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "emission" } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "lighting", GET_RESOURCE_PATH( "shaders/deferred/composition.glsl" ), compBindings );

    std::unordered_map<std::string, Render::ShaderProgram::UniformBinding> toneBindings = {
        { "u_input", { .set = 0, .binding = 0, .type = UniformType::ImageSampler, .source = BindingSource::Attachment, .resourceName = "lighting" } },
    };
    m_graph.register_shader<Render::GraphicShaderProgram>( "tonemapping", GET_RESOURCE_PATH( "shaders/misc/tonemapping.glsl" ), toneBindings );
}

void DeferredRenderer2::configure_passes() {

    // Main configs
    const Extent2D        DISPLAY_EXTENT = !m_headless ? m_window->get_extent() : m_headlessExtent;
    const ColorFormatType HDR_FORMAT     = m_settings.highDynamicPrecission == FloatPrecission::F16 ? SRGBA_16F : SRGBA_32F;
    const ColorFormatType DEPTH_FORMAT   = m_settings.depthPrecission == FloatPrecission::F16 ? DEPTH_16F : DEPTH_32F;
    const Extent2D        SHADOW_RES     = { (uint32_t)m_settings.shadowQuality, (uint32_t)m_settings.shadowQuality };
    const Extent2D        SKY_RES        = { 1024, 512 };

    m_graph.add_pass( "ShadowPass", { "shadows" }, [&]( Render::RenderGraphBuilder& builder ) {
            builder.create_target("shadows", Render::TargetInfo{
                .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .clearValue = {.color = {{0, 0, 0, 1}}},
                .load = false,
                .store = true
            }); }, [&]( const RenderView& view, const RenderResources& shared, const RenderPassOutputs& outputs ) {

        auto* program = graph.get_shader_program( "Lighting" );
        // Bind pipeline and descriptor set, draw fullscreen quad
    } );

    graph.add_pass( "LightingPass", graph.get_shader_program( "Lighting" ), [&]( Render::RenderGraphBuilder& builder ) {
            builder.read("depth");
            builder.read("albedo");
            builder.create_target("lighting", RenderTargetInfo{
                .format = VK_FORMAT_R16G16B16A16_SFLOAT,
                .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .clearValue = {.color = {{0, 0, 0, 1}}},
                .load = false,
                .store = true
            }); }, [&]( const RenderView& view, const RenderResources& shared, const RenderPassOutputs& outputs ) {
        auto* program = graph.get_shader_program( "Lighting" );
         PROFILING_EVENT()

    CommandBuffer cmd = view.commandBuffer;

    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);

    ShaderPass* shaderPass = m_shaderPasses["composition"];

    cmd.bind_shaderpass(*shaderPass);

    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &m_settings, sizeof(Settings));
    cmd.bind_descriptor_set(m_descriptors[view.frameIndex].globalDescritor, 0, *shaderPass, {0, 0});
    cmd.bind_descriptor_set(m_descriptors[view.frameIndex].gBufferDescritor, 1, *shaderPass);

    cmd.draw_geometry(*get_VAO(shared.vignette));

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);
    } );

    // Shadows
    // Geometry
    // Lighting
    // Tonemapping
}

} // namespace Systems
VULKAN_ENGINE_NAMESPACE_END
