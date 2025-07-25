#include <engine/render/passes/precomposition_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Render {

void PreCompositionPass::create_samples_kernel() {

    std::uniform_real_distribution<GLfloat> randomFloats( 0.0, 1.0 );
    std::default_random_engine              generator;

    std::vector<Vec4> AOSampleKernel;
    AOSampleKernel.resize( MAX_KERNEL_MEMBERS );

    for ( unsigned int i = 0; i < m_AO.samples; ++i )
    {
        Vec4 sample( randomFloats( generator ) * 2.0 - 1.0, randomFloats( generator ) * 2.0 - 1.0, randomFloats( generator ), 0.0f );
        sample = math::normalize( sample );
        sample *= randomFloats( generator );
        float scale = float( i ) / float( m_AO.samples );

        auto lerp = []( float a, float b, float f ) { return a + f * ( b - a ); };

        // Center importance sampling
        scale = lerp( 0.1f, 1.0f, scale * scale );
        sample *= scale;
        AOSampleKernel[i] = sample;
    }

    m_kernelBuffer.upload_data( AOSampleKernel.data(), m_kernelBuffer.size );
    m_updateSamplesKernel = false;
}

void PreCompositionPass::setup_out_attachments( std::vector<Graphics::AttachmentConfig>& attachments, std::vector<Graphics::SubPassDependency>& dependencies ) {

    attachments.resize( 1 );

    // SSAO and RT SHADOWS buffer
    attachments[0] = Graphics::AttachmentConfig( RG_8U,
                                                 1,
                                                 LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                 LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                 IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC | IMAGE_USAGE_TRANSFER_DST,
                                                 COLOR_ATTACHMENT,
                                                 ASPECT_COLOR,
                                                 TEXTURE_2D,
                                                 FILTER_LINEAR,
                                                 ADDRESS_MODE_CLAMP_TO_EDGE );

    // Depdencies
    dependencies.resize( 2 );

    dependencies[0]               = Graphics::SubPassDependency( STAGE_FRAGMENT_SHADER, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE );
    dependencies[0].srcAccessMask = ACCESS_SHADER_READ;
    dependencies[1]               = Graphics::SubPassDependency( STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_FRAGMENT_SHADER, ACCESS_SHADER_READ );
    dependencies[1].srcAccessMask = ACCESS_COLOR_ATTACHMENT_WRITE;
    dependencies[1].srcSubpass    = 0;
    dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
}

void PreCompositionPass::create_framebuffer() {
    m_interAttachments.resize( 1 );
    std::vector<Graphics::Image*> out1 = { &m_interAttachments[0] };
    std::vector<Graphics::Image*> out2 = { m_outAttachments[0] };
    m_framebuffers[0]                  = m_device->create_framebuffer( m_renderpass, out1, m_imageExtent, m_framebufferImageDepth, 0 );
    m_framebuffers[1]                  = m_device->create_framebuffer( m_renderpass, out2, m_imageExtent, m_framebufferImageDepth, 1 );
}

void PreCompositionPass::setup_uniforms( std::vector<Graphics::Frame>& frames ) {

    /////////////////////////////////////////////////
    // CREATE INTERNAL SSAO UNIFORM DATA
    /////////////////////////////////////////////////
    const size_t BUFFER_SIZE = m_device->pad_uniform_buffer_size( sizeof( Vec4 ) * MAX_KERNEL_MEMBERS );
    m_kernelBuffer           = m_device->create_buffer_VMA( BUFFER_SIZE, BUFFER_USAGE_UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU );

    // AO KERNEL BUFFER -----------
    create_samples_kernel();

    /////////////////////////////////////////////////
    /////////////////////////////////////////////////

    m_descriptorPool = m_device->create_descriptor_pool( 10, 10, 10, 10, 10 );
    m_descriptors.resize( frames.size() );
    // GLOBAL
    LayoutBinding camBufferBinding( UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_FRAGMENT, 0 );
    LayoutBinding sceneBufferBinding( UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_FRAGMENT, 1 );
    // G- BUFFER
    LayoutBinding positionBinding( UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2 );
    LayoutBinding normalBinding( UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3 );
    // SSAO
    LayoutBinding AOkernelBinding( UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 4 );
    // RTX
    LayoutBinding noiseBinding( UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5 );
    LayoutBinding accelBinding( UNIFORM_ACCELERATION_STRUCTURE, SHADER_STAGE_FRAGMENT, 6 );

    m_descriptorPool.set_layout(
        GLOBAL_LAYOUT, { camBufferBinding, sceneBufferBinding, positionBinding, normalBinding, AOkernelBinding, noiseBinding, accelBinding } );

    // TO BLUR IMAGE LAYOUT
    LayoutBinding toBlurImageBinding( UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0 );
    m_descriptorPool.set_layout( 1, { toBlurImageBinding } );

    for ( size_t i = 0; i < frames.size(); i++ )
    {
        m_descriptorPool.allocate_descriptor_set( GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor );
        m_descriptorPool.allocate_descriptor_set( 1, &m_descriptors[i].blurImageDescritor );

        m_descriptors[i].globalDescritor.update( &frames[i].uniformBuffers[GLOBAL_LAYOUT], sizeof(  Core::Camera::GPUPayload ), 0, UNIFORM_DYNAMIC_BUFFER, 0 );
        m_descriptors[i].globalDescritor.update( &frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                                 sizeof(  Core::Scene::GPUPayload ),
                                                 m_device->pad_uniform_buffer_size( sizeof(  Core::Camera::GPUPayload ) ),
                                                 UNIFORM_DYNAMIC_BUFFER,
                                                 1 );
        m_descriptors[i].globalDescritor.update( &m_kernelBuffer, BUFFER_SIZE, 0, UNIFORM_BUFFER, 4 );

        m_descriptors[i].globalDescritor.update(
            m_shared->get_image_resource( "BlueNoise" ), LAYOUT_SHADER_READ_ONLY_OPTIMAL, 5 );
    }
}
void PreCompositionPass::setup_shader_passes() {

    GraphicShaderPass* compPass =
        new GraphicShaderPass( m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH( "shaders/deferred/pre_composition.glsl" ) );
    compPass->settings.descriptorSetLayoutIDs = { { GLOBAL_LAYOUT, true } };
    compPass->graphicSettings.attributes      = {
        { POSITION_ATTRIBUTE, true }, { NORMAL_ATTRIBUTE, false }, { UV_ATTRIBUTE, true }, { TANGENT_ATTRIBUTE, false }, { COLOR_ATTRIBUTE, false } };

    compPass->settings.pushConstants = { PushConstant( SHADER_STAGE_FRAGMENT, sizeof( AO ) ) };

    compPass->build_shader_stages();
    compPass->build( m_descriptorPool );
    m_shaderPasses["pre"] = compPass;

    GraphicShaderPass* blurPass =
        new GraphicShaderPass( m_device->get_handle(), m_renderpass, m_imageExtent, GET_RESOURCE_PATH( "shaders/misc/bilateral_filter.glsl" ) );
    blurPass->settings.descriptorSetLayoutIDs = { { 0, true }, { 1, true } };
    blurPass->graphicSettings.attributes      = {
        { POSITION_ATTRIBUTE, true }, { NORMAL_ATTRIBUTE, false }, { UV_ATTRIBUTE, true }, { TANGENT_ATTRIBUTE, false }, { COLOR_ATTRIBUTE, false } };

    blurPass->settings.pushConstants = { PushConstant( SHADER_STAGE_FRAGMENT, sizeof( float ) ) };

    blurPass->build_shader_stages();
    blurPass->build( m_descriptorPool );
    m_shaderPasses["blur"] = blurPass;
}

void PreCompositionPass::execute( Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex ) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;

    cmd.begin_renderpass( m_renderpass, m_framebuffers[0] );
    cmd.set_viewport( m_imageExtent );

    ShaderPass* shaderPass = m_shaderPasses["pre"];

    cmd.bind_shaderpass( *shaderPass );

    cmd.push_constants( *shaderPass, SHADER_STAGE_FRAGMENT, &m_AO, sizeof( AO ) );
    cmd.bind_descriptor_set( m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, { 0, 0 } );

    cmd.draw_geometry( m_shared->get_vignette_VAO() );

    cmd.end_renderpass( m_renderpass, m_framebuffers[0] );

    // /////////////////////////////////////////
    // /*Copy data to tmp previous frame image*/
    // /////////////////////////////////////////

    cmd.begin_renderpass( m_renderpass, m_framebuffers[1] );
    cmd.set_viewport( m_imageExtent );

    shaderPass = m_shaderPasses["blur"];

    cmd.bind_shaderpass( *shaderPass );

    // struct BFilterUniforms {
    //     float kernel;
    //     float sigmaA;
    //     float sigmaB;
    // };

    // BFilterUniforms filter;
    // filter.kernel = m_AO.blurRadius;
    // filter.sigmaA = m_AO.blurSigmaA;
    // filter.sigmaB = m_AO.blurSigmaB;

    cmd.push_constants( *shaderPass, SHADER_STAGE_FRAGMENT, &m_AO.blurRadius, sizeof( float ) );
    cmd.bind_descriptor_set( m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, { 0, 0 } );
    cmd.bind_descriptor_set( m_descriptors[currentFrame.index].blurImageDescritor, 1, *shaderPass );

    cmd.draw_geometry( m_shared->get_vignette_VAO() );

    cmd.end_renderpass( m_renderpass, m_framebuffers[1] );
}
void PreCompositionPass::link_input_attachments() {
    for ( size_t i = 0; i < m_descriptors.size(); i++ )
    {
        // SET UP G-BUFFER
        m_descriptors[i].globalDescritor.update( m_inAttachments[0], LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, 2 ); // POSITION
        m_descriptors[i].globalDescritor.update( m_inAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 3 );        // NORMALS
        // RAW SSAO
        m_descriptors[i].blurImageDescritor.update( &m_interAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0 );
    }
}

void PreCompositionPass::update_uniforms( uint32_t frameIndex, Scene* const scene ) {
    if ( m_updateSamplesKernel )
        create_samples_kernel();

    // Set STATIC top level accel. structure
    for ( size_t i = 0; i < m_descriptors.size(); i++ )
    {
        m_descriptors[i].globalDescritor.update( get_TLAS( scene ), 6 );
    }
}

void PreCompositionPass::cleanup() {
    m_kernelBuffer.cleanup();
    BaseGraphicPass::cleanup();
}

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END