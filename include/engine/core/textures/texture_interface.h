/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_CORE_H
#define TEXTURE_CORE_H

#include <engine/graphics/descriptors.h>
#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Render {
class RenderViewBuilder;
}

namespace Core {

struct SamplingConfig {
    uint32_t    maxMipLevel        = 1U;
    uint32_t    baseMipLevel       = 0;
    FilterType  filters            = FILTER_LINEAR;
    MipmapMode  mipmapMode         = MIPMAP_LINEAR;
    AddressMode samplerAddressMode = ADDRESS_MODE_REPEAT;
    float       maxAnysotropy      = 1.0f;
    BorderColor border             = BorderColor::FLOAT_OPAQUE_WHITE;
};

/*
Interface class for all textures
*/
class ITexture
{
protected:
    std::string    m_fileRoute = "";
    Extent3D       m_size      = {};
    uint16_t       m_channels  = 0;
    SamplingConfig m_sampling  = {};

    Graphics::Texture m_handle = {};

    bool m_isDirty { true };
    bool m_loadedOnClient { false };

    friend class Render::RenderViewBuilder;

public:
    ITexture() {
    }

    ITexture( SamplingConfig settings )
        : m_sampling( settings ) {
    }

    ITexture( Extent3D       size,
              uint16_t       channels,
              SamplingConfig settings = {} )
        : m_size( size )
        , m_sampling( settings )
        , m_channels( channels ) {
    }

    virtual void   set_image_cache( void* cache, Extent3D extent, uint16_t channels ) = 0;
    virtual void   get_image_cache( void*& cache ) const                              = 0;
    virtual size_t get_bytes_per_pixel() const                                        = 0;

    // GETTERS & SETTERS
    inline size_t         get_channels() const { return m_channels; };
    inline Extent3D       get_size() const { return m_size; };
    inline bool           loaded_on_CPU() const { return m_loadedOnClient; }
    inline bool           loaded_on_GPU() const { return m_handle.image; }
    inline bool           is_dirty() const { return m_isDirty; }
    inline void           set_dirty( bool d ) { m_isDirty = d; }
    inline SamplingConfig get_settings() const { return m_sampling; }
    inline void           set_settings( SamplingConfig settings ) { m_sampling = settings; }
    inline std::string    get_file_route() const { return m_fileRoute; }
    inline void           set_file_route( std::string r ) { m_fileRoute = r; }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif
