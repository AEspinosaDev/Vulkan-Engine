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

typedef Graphics::TextureConfig TextureSettings ;

/*
Interface class for all textures
*/
class ITexture
{
  protected:
    TextureSettings m_settings = {};

    Graphics::Texture m_handle    = {};
    uint16_t          m_channels  = 0;
    std::string       m_fileRoute = "";

    bool m_isDirty{true};
    bool m_loadedOnClient{false};

    friend class Render::RenderViewBuilder;

  public:
    ITexture() {
        m_handle.image = new Graphics::Image();
    }

    ITexture(TextureSettings settings)
        : m_settings(settings) {
        m_handle.image = new Graphics::Image();
    }

    ITexture(uint16_t channels, TextureSettings settings = {})
        : m_settings(settings)
        , m_channels(channels) {
        m_handle.image = new Graphics::Image();
    }

    virtual void   set_image_cache(void* cache, Extent3D extent, uint16_t channels) = 0;
    virtual void   get_image_cache(void*& cache) const                              = 0;
    virtual size_t get_bytes_per_pixel() const                                      = 0;

    // GETTERS & SETTERS
    inline size_t get_channels() const {
        return m_channels;
    };

    inline bool loaded_on_CPU() const {
        return m_loadedOnClient;
    }

    inline bool loaded_on_GPU() const {
        return !m_handle.image->empty;
    }

    inline bool is_dirty() const {
        return m_isDirty;
    }

    inline void set_dirty(bool d) {
        m_isDirty = d;
    }

    inline TextureSettings get_settings() const {
        return m_settings;
    }

    inline void set_settings(TextureSettings settings) {
        m_settings = settings;
    }

    inline Extent3D get_size() const {
        return m_image.extent;
    }
    inline void set_size(Extent3D s) {
        m_image.extent = s;
    }

    inline std::string get_file_route() const {
        return m_fileRoute;
    }
    inline void set_file_route(std::string r) {
        m_fileRoute = r;
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif
