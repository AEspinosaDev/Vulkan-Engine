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

namespace Core {

struct TextureSettings {
    TextureType     type              = TEXTURE_2D;
    ColorFormatType format            = SRGBA_8;
    FilterType      filter            = FILTER_LINEAR;
    AddressMode     adressMode        = ADDRESS_MODE_REPEAT;
    bool            useMipmaps        = true;
    uint16_t        anisotropicFilter = 16;
    int             minMipLevel       = 0;
    int             maxMipLevel       = 12;
};

/*
Interface class for all textures
*/
class ITexture
{
  protected:
    TextureSettings m_settings = {};

    Graphics::Image m_image     = {};
    uint16_t        m_channels  = 0;
    std::string     m_fileRoute = "None";

    bool m_isDirty{true};

    friend Graphics::Image* const get_image(ITexture* t);

  public:
    ITexture() {
        m_image.extent = {0, 0, 1};
    }

    ITexture(TextureSettings settings)
        : m_settings(settings) {
        m_image.extent = {0, 0, 1};
    }

    ITexture(Extent3D size, uint16_t channels, TextureSettings settings = {})
        : m_settings(settings)
        , m_channels(channels) {
        m_image.extent = size;
    }

    virtual inline void set_image_cache(void* cache, Extent3D extent, uint16_t channels) = 0;

    virtual inline void get_image_cache(void*& cache) const = 0;

    virtual inline size_t get_bytes_per_pixel() const = 0;

    // GETTERS & SETTERS

    inline bool loaded_on_CPU() const {
        return m_image.loadedOnCPU;
    }

    inline bool loaded_on_GPU() const {
        return m_image.loadedOnGPU;
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

    inline void set_use_mipmaps(bool op) {
        m_settings.useMipmaps = op;
    }

    inline void set_anysotropic_filtering(bool op) {
        m_settings.anisotropicFilter = op;
    }

    inline void set_format(ColorFormatType f) {
        m_settings.format = f;
    }

    inline void set_filter(FilterType f) {
        m_settings.filter = f;
    }

    inline void set_adress_mode(AddressMode am) {
        m_settings.adressMode = am;
    }
    inline void set_type(TextureTypeFlagBits t) {
        m_settings.type = t;
    }
    inline TextureTypeFlagBits get_type() const {
        return m_settings.type;
    }
    inline std::string get_file_route() const {
        return m_fileRoute;
    }
    inline void set_file_route(std::string r) {
        m_fileRoute = r;
    }
};

Graphics::Image* const get_image(ITexture* t);

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif
