/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_H
#define TEXTURE_H

#include <engine/graphics/descriptors.h>
#include <engine/graphics/image.h>
#include <stb_image.h>
#include <thread>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct TextureSettings
{
    ColorFormatType format{SRGBA_8};
    TextureFilterType filter{LINEAR};
    TextureAdressModeType adressMode{REPEAT};

    bool useMipmaps{true};
    bool anisotropicFilter{true};

    int minMipLevel{0};
    int maxMipLevel{-1};
};

class Texture
{
    TextureSettings m_settings{};

    Image m_image{};

    bool m_isDirty{true};

    friend Image *const get_image(Texture *t);

  public:
    static Texture *DEBUG_TEXTURE;

    Texture()
    {
        m_image.extent = {0, 0, 1};
    }
    Texture(TextureSettings settings) : m_settings(settings)
    {
        m_image.extent = {0, 0, 1};
    }
    Texture(unsigned char *data, Extent3D size, TextureSettings settings = {}) : m_settings(settings)
    {
        m_image.tmpCache = data;
        m_image.extent = size;
    }

    inline bool data_loaded() const
    {
        return m_image.loadedOnCPU;
    }
    inline bool is_buffer_loaded() const
    {
        return m_image.loadedOnGPU;
    }
    inline bool is_dirty() const
    {
        return m_isDirty;
    }
    inline void set_dirty(bool d)
    {
        m_isDirty = d;
    }

    inline TextureSettings get_settings() const
    {
        return m_settings;
    }
    inline void set_settings(TextureSettings settings)
    {
        m_settings = settings;
    }

    inline Extent3D get_size() const
    {
        return m_image.extent;
    }

    inline void set_image_cache(unsigned char *cache, Extent2D extent, unsigned int channels)
    {
        m_image.tmpCache = cache;
        m_image.extent = {extent.width, extent.height, 1};
        m_image.loadedOnCPU = true;
        m_isDirty = true;
    }

    inline void set_use_mipmaps(bool op)
    {
        m_settings.useMipmaps = op;
    }
    inline void set_anysotropic_filtering(bool op)
    {
        m_settings.anisotropicFilter = op;
    }
    inline void set_format(ColorFormatType f)
    {
        m_settings.format = f;
    }
    inline void set_filter(TextureFilterType f)
    {
        m_settings.filter = f;
    }
    inline void set_adress_mode(TextureAdressModeType am)
    {
        m_settings.adressMode = am;
    }
};

Image *const get_image(Texture *t);
VULKAN_ENGINE_NAMESPACE_END

#endif