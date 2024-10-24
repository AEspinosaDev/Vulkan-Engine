/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_H
#define TEXTURE_H

#include <engine/graphics/descriptors.h>
#include <engine/graphics/image.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

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

// Type aliases for specific texture types
template <typename T> class TextureBase;
using Texture = TextureBase<unsigned char>; // Basic texture. Support for LDR data
using TextureHDR = TextureBase<float>;      // Texture with support for HDR data

/*
Template class for all textures
*/
template <typename T> class TextureBase
{
    static_assert(std::is_same<T, unsigned char>::value || std::is_same<T, float>::value,
                  "TextureBase can only be instantiated with unsigned char or float.");

  protected:
    T *m_tmpCache{nullptr};

    TextureSettings m_settings{};

    Graphics::Image m_image{};

    uint16_t m_channels{0};

    bool m_isDirty{true};

    friend Graphics::Image *const get_image(TextureBase<unsigned char> *t);
    friend Graphics::Image *const get_image(TextureBase<float> *t);

  public:
    static TextureBase<unsigned char> *DEBUG_TEXTURE;

    TextureBase()
    {
        m_image.extent = {0, 0, 1};
    }

    TextureBase(TextureSettings settings) : m_settings(settings)
    {
        m_image.extent = {0, 0, 1};
    }

    TextureBase(T *cache, Extent3D size, uint16_t channels, TextureSettings settings = {})
        : m_settings(settings), m_channels(channels)
    {
        m_tmpCache = cache;
        m_image.extent = size;
    }

    inline void set_image_cache(void *cache, Extent3D extent, uint16_t channels)
    {
        m_tmpCache = static_cast<T *>(cache); // Cast cache to type T
        m_channels = channels;                // Set the number of channels
        m_image.extent = extent;              // Set the image extent
        m_image.loadedOnCPU = true;           // Mark the image as loaded on CPU
        m_isDirty = true;                     // Mark as dirty
    }

    inline void get_image_cache(void *&cache) const
    {
        cache = static_cast<void *>(m_tmpCache); // Cast m_tmpCache to void*
    }

    inline size_t get_bytes_per_pixel() const
    {
        return static_cast<size_t>(m_channels) * sizeof(T); // Calculate bytes per pixel based on type T
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



 Graphics::Image *const get_image(TextureBase<unsigned char> *t);
 Graphics::Image *const get_image(TextureBase<float> *t);

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif
