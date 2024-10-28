/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_HDR_H
#define TEXTURE_HDR_H

#include <engine/core/textures/texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{
/*
 Texture with support for HDR data
*/
class TextureHDR : public ITexture
{
  private:
    float *m_tmpCache{nullptr};

  public:
    TextureHDR() : ITexture()
    {
    }
    TextureHDR(TextureSettings settings) : ITexture(settings)
    {
    }
    TextureHDR(float *data, Extent3D size, uint16_t channels, TextureSettings settings = {})
        : ITexture(size, channels, settings), m_tmpCache(data)
    {
    }

    inline void set_image_cache(void *cache, Extent3D extent, uint16_t channels)
    {
        m_tmpCache = static_cast<float *>(cache);
        m_channels = channels;      // Set the number of channels
        m_image.extent = extent;    // Set the image extent
        m_image.loadedOnCPU = true; // Mark the image as loaded on CPU
        m_isDirty = true;           // Mark as dirty
    }
    inline void get_image_cache(void *&cache) const
    {
        cache = static_cast<void *>(m_tmpCache);
    }
    inline size_t get_bytes_per_pixel() const
    {
        return static_cast<size_t>(m_channels) * sizeof(float);
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif