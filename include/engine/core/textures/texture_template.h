/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_CORE_TEMPLATE_H
#define TEXTURE_CORE_TEMPLATE_H

#include <engine/core/textures/texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

enum class DynamicRange
{
    LDR,   // uint8_t (non-normalized)
    UNORM, // uint8_t [0,1]
    HDR    // float32
};

enum class ChannelFormat
{
    R    = 1,
    RG   = 2,
    RGB  = 3,
    RGBA = 4
};

enum class ColorEncoding
{
    Linear, // Linear space (default)
    SRGB    // sRGB gamma encoded
};

template <DynamicRange DR>
struct PixelTraits;

template <>
struct PixelTraits<DynamicRange::LDR> {
    using Type                         = uint8_t;
    static constexpr bool IsNormalized = false;
};

template <>
struct PixelTraits<DynamicRange::UNORM> {
    using Type                         = uint8_t;
    static constexpr bool IsNormalized = true;
};

template <>
struct PixelTraits<DynamicRange::HDR> {
    using Type                         = float;
    static constexpr bool IsNormalized = false;
};

template <DynamicRange DR, ChannelFormat CF, ColorEncoding CE = ColorEncoding::Linear>
class Texture : public ITexture
{
public:
    using ElementType                  = typename PixelTraits<DR>::Type;
    static constexpr bool IsNormalized = PixelTraits<DR>::IsNormalized;
    static constexpr bool IsSRGB       = ( CE == ColorEncoding::SRGB );

private:
    ElementType* m_cache { nullptr };

public:
    Texture()
        : ITexture() {}

    Texture( TextureSettings settings )
        : ITexture( settings ) {}

    Texture( ElementType* data, Extent3D size, TextureSettings settings = {} )
        : ITexture( size, static_cast<uint16_t>( CF ), settings )
        , m_cache( data ) {
        m_image.loadedOnCPU = true;
    }

    inline void set_image_cache( void* cache, Extent3D extent ) override {
        m_cache             = static_cast<ElementType*>( cache );
        m_image.extent      = extent;
        m_image.loadedOnCPU = true;
        m_isDirty           = true;
    }

    inline void get_image_cache( void*& cache ) const override {
        cache = static_cast<void*>( m_cache );
    }

    inline size_t get_bytes_per_pixel() const override {
        return static_cast<size_t>( m_channels ) * sizeof( ElementType );
    }

    inline bool is_srgb() const {
        return IsSRGB;
    }
};

using TextureRGBA      = Texture<DynamicRange::LDR, ChannelFormat::RGBA>;
using TextureUnormRGBA = Texture<DynamicRange::UNORM, ChannelFormat::RGBA>;
using TextureSRGBA     = Texture<DynamicRange::UNORM, ChannelFormat::RGBA, ColorEncoding::SRGB>;
using TextureFloatRGBA = Texture<DynamicRange::HDR, ChannelFormat::RGBA>;
using TextureRGB       = Texture<DynamicRange::LDR, ChannelFormat::RGB>;
using TextureUnormRGB  = Texture<DynamicRange::UNORM, ChannelFormat::RGB>;
using TextureSRGB      = Texture<DynamicRange::UNORM, ChannelFormat::RGB, ColorEncoding::SRGB>;
using TextureFloatRGB  = Texture<DynamicRange::HDR, ChannelFormat::RGB>;

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif