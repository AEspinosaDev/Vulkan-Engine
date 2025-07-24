#include <engine/render/GPU_resource_pool.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Render::GPUResourcePool::init( const std::shared_ptr<Graphics::Device>& device ) {
    m_device = device;

    // Init basic resources
    auto vignette = Core::Geometry::create_quad();
    upload_geometry_data( device, vignette, false );
    m_vignetteVAO = *get_VAO(vignette);

    unsigned char texture_data[1]   = { 0 };
    auto          FallbackTexture2D = new Core::TextureLDR( texture_data, { 1, 1, 1 }, 4 );
    FallbackTexture2D->set_use_mipmaps( false );
    upload_texture_data( device, FallbackTexture2D );
    m_fallbackImage2D = *get_image( FallbackTexture2D );
    delete FallbackTexture2D;

    unsigned char cube_data[6] = { 0, 0, 0, 0, 0, 0 };
    auto          FallbackCube = new Core::TextureLDR( cube_data, { 1, 1, 1 }, 4 );
    FallbackCube->set_use_mipmaps( false );
    FallbackCube->set_type( TextureTypeFlagBits::TEXTURE_CUBE );
    upload_texture_data( device, FallbackCube );
    m_fallbackCubemap = *get_image( FallbackCube );
    delete FallbackCube;
}
void Render::GPUResourcePool::cleanup() {
    m_vignetteVAO.ibo.cleanup();
    m_vignetteVAO.vbo.cleanup();
    m_fallbackCubemap.cleanup();
    m_fallbackImage2D.cleanup();
    m_fallbackImage3D.cleanup();

    for ( auto& [name, buffer] : m_ubos )
    {
        buffer.cleanup();
    }
    m_ubos.clear();

    for ( auto& [name, image] : m_images )
    {
        image.cleanup();
    }
    m_images.clear();
}

void Render::GPUResourcePool::register_image( const std::string& name, Core::ITexture* const t ) {

    upload_texture_data( m_device, t );
    m_images[name] = *get_image( t );
}

void Render::GPUResourcePool::upload_texture_data( const ptr<Graphics::Device>& device, Core::ITexture* const t ) {
    if ( t && t->loaded_on_CPU() )
    {
        if ( !t->loaded_on_GPU() )
        {
            Graphics::ImageConfig   config        = {};
            Graphics::SamplerConfig samplerConfig = {};
            Core::TextureSettings   textSettings  = t->get_settings();
            config.viewType                       = textSettings.type;
            config.format                         = textSettings.format;
            config.mipLevels                      = textSettings.useMipmaps ? textSettings.maxMipLevel : 1;
            samplerConfig.anysotropicFilter       = textSettings.anisotropicFilter;
            samplerConfig.filters                 = textSettings.filter;
            samplerConfig.maxLod                  = textSettings.maxMipLevel;
            samplerConfig.minLod                  = textSettings.minMipLevel;
            samplerConfig.samplerAddressMode      = textSettings.adressMode;

            void* imgCache { nullptr };
            t->get_image_cache( imgCache );
            device->upload_texture_image( *get_image( t ), config, samplerConfig, imgCache, t->get_bytes_per_pixel() );
        }
    }
}

void Render::GPUResourcePool::destroy_texture_data( Core::ITexture* const t ) {
    if ( t )
        get_image( t )->cleanup();
}
void Render::GPUResourcePool::upload_geometry_data( const ptr<Graphics::Device>& device, Core::Geometry* const g, bool createAccelStructure ) {
    PROFILING_EVENT()
    /*
    VERTEX ARRAYS
    */
    Graphics::VertexArrays* rd = get_VAO( g );
    if ( !rd->loadedOnGPU )
    {
        const Core::GeometricData& gd        = g->get_properties();
        size_t                     vboSize   = sizeof( gd.vertexData[0] ) * gd.vertexData.size();
        size_t                     iboSize   = sizeof( gd.vertexIndex[0] ) * gd.vertexIndex.size();
        size_t                     voxelSize = sizeof( gd.voxelData[0] ) * gd.voxelData.size();
        rd->indexCount                       = gd.vertexIndex.size();
        rd->vertexCount                      = gd.vertexData.size();
        rd->voxelCount                       = gd.voxelData.size();

        device->upload_vertex_arrays( *rd, vboSize, gd.vertexData.data(), iboSize, gd.vertexIndex.data(), voxelSize, gd.voxelData.data() );
    }
    /*
    ACCELERATION STRUCTURE
    */
    if ( createAccelStructure )
    {
        Graphics::BLAS* accel = get_BLAS( g );
        if ( !accel->handle )
            device->upload_BLAS( *accel, *get_VAO( g ) );
    }
}
void Render::GPUResourcePool::destroy_geometry_data( Core::Geometry* const g ) {
    Graphics::VertexArrays* rd = get_VAO( g );
    if ( rd->loadedOnGPU )
    {
        rd->vbo.cleanup();
        if ( rd->indexCount > 0 )
            rd->ibo.cleanup();
        if ( rd->voxelCount > 0 )
            rd->voxelBuffer.cleanup();

        rd->loadedOnGPU = false;
        get_BLAS( g )->cleanup();
    }
}
VULKAN_ENGINE_NAMESPACE_END