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

template <typename T> class Texture;
typedef Texture<unsigned char> TextureLDR; /* Standard Low Dynamic Range */
typedef Texture<float>         TextureHDR; /* High Dynamic Range */

template <typename T> class Texture final : public ITexture
{
  private:
    T* m_chache{nullptr};

  public:
    Texture()
        : ITexture() {
    }
    Texture(TextureSettings settings)
        : ITexture(settings) {
    }
    Texture(T* data, Extent3D size, uint16_t channels, TextureSettings settings = {})
        : ITexture(size, channels, settings)
        , m_chache(data) {
        m_image.loadedOnCPU = true;
    }

    inline void set_image_cache(void* cache, Extent3D extent, uint16_t channels) override {
        m_chache            = static_cast<T*>(cache);
        m_channels          = channels; // Set the number of channels
        m_image.extent      = extent;   // Set the image extent
        m_image.loadedOnCPU = true;     // Mark the image as loaded on CPU
        m_isDirty           = true;     // Mark as dirty
    }
    inline void get_image_cache(void*& cache) const override {
        cache = static_cast<void*>(m_chache);
    }
    inline size_t get_bytes_per_pixel() const override {
        return static_cast<size_t>(m_channels) * sizeof(T);
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif