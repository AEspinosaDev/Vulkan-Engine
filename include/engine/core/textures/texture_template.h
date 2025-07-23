/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_CORE_TEMPLATE_H
#define TEXTURE_CORE_TEMPLATE_H

#include <engine/core/textures/texture_interface.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

enum class DynamicRange
{
    Uint,  // uint8_t / char
    Unorm, // uint8_t / normalized
    Float, // float32
};

enum class ColorEncoding
{
    Linear, // Linear space (default)
    SRGB    // sRGB gamma encoded
};

template <DynamicRange DR>
struct PixelTraits;

template <>
struct PixelTraits<DynamicRange::Uint> {
    using Type                         = uint8_t;
    static constexpr bool IsNormalized = false;
};

template <>
struct PixelTraits<DynamicRange::Unorm> {
    using Type                         = uint8_t;
    static constexpr bool IsNormalized = true;
};

template <>
struct PixelTraits<DynamicRange::Float> {
    using Type                         = float;
    static constexpr bool IsNormalized = false;
};

template <DynamicRange DR, TextureType T, ColorEncoding CE = ColorEncoding::Linear>
class TextureTemplate : public ITexture
{
public:
    using ElementType                  = typename PixelTraits<DR>::Type;
    static constexpr bool IsNormalized = PixelTraits<DR>::IsNormalized;
    static constexpr bool IsSRGB       = ( CE == ColorEncoding::SRGB );

private:
    ElementType* m_cache { nullptr };

public:
    TextureTemplate()
        : ITexture() {}

    TextureTemplate( SamplingConfig settings )
        : ITexture( settings ) {}

    TextureTemplate( ElementType*   data,
             Extent3D       size,
             uint16_t       channels,
             TextureType    type     = TextureType::TEXTURE_2D,
             SamplingConfig settings = {} )
        : ITexture( size, channels, settings )
        , m_cache( data ) {
        m_loadedOnClient = true;
    }

    inline void set_image_cache( void* cache, Extent3D extent, uint16_t channels ) override {
        m_cache          = static_cast<ElementType*>( cache );
        m_channels       = channels;
        m_loadedOnClient = true;
        m_isDirty        = true;
    }

    inline void get_image_cache( void*& cache ) const override {
        cache = static_cast<void*>( m_cache );
    }

    inline size_t get_bytes_per_pixel() const override {
        return static_cast<size_t>( m_channels ) * sizeof( ElementType );
    }

    inline bool is_SRGB() const {
        return IsSRGB;
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif