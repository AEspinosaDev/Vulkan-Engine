/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_LDR_H
#define TEXTURE_LDR_H

#include <engine/core/textures/texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{
class TextureLDR;
typedef TextureLDR Texture;
/*
Standard texture that supports linear color data from standard image files (jpg,png etc)
*/
class TextureLDR : public TextureBase
{
  private:
    unsigned char *m_tmpCache{nullptr};

  public:
    static Texture *DEBUG_TEXTURE;

    TextureLDR() : TextureBase()
    {
    }
    TextureLDR(TextureSettings settings) : TextureBase(settings)
    {
    }
    TextureLDR(unsigned char *data, Extent3D size, uint16_t channels, TextureSettings settings = {})
        : TextureBase(size, channels, settings), m_tmpCache(data)
    {
    }

    inline void set_image_cache(void *cache, Extent3D extent, uint16_t channels)
    {
        m_tmpCache = static_cast<unsigned char *>(cache);
        m_channels = channels;
        m_image.extent = extent;
        m_image.loadedOnCPU = true;
        m_isDirty = true;
    }
    inline void get_image_cache(void *&cache) const
    {
        cache = m_tmpCache;
    }
    inline size_t get_bytes_per_pixel() const
    {
        return static_cast<unsigned long long>(m_channels) * sizeof(unsigned char);
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif