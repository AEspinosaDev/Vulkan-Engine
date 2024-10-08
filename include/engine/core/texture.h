/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef TEXTURE_H
#define TEXTURE_H

#include <stb_image.h>
#include <thread>
#include <engine/graphics/image.h>
#include <engine/graphics/descriptors.h>

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
    unsigned char *m_tmpCache;
    int m_channels;

    Image m_image;

    TextureSettings m_settings{};

    bool m_loaded{false};
    bool m_buffer_loaded{false};
    bool m_isDirty{true};

    friend class Renderer;

public:
    static Texture *DEBUG_TEXTURE;

    Texture() : m_tmpCache(nullptr) { m_image.extent = {0, 0, 1}; }
    Texture(TextureSettings settings) : m_settings(settings), m_tmpCache(nullptr) { m_image.extent = {0, 0, 1}; }
    Texture(unsigned char *data, Extent3D size, TextureSettings settings = {}) : m_settings(settings), m_tmpCache(data)
    {
        m_image.extent = size;
    }

    inline bool data_loaded() const { return m_loaded; }
    inline bool is_buffer_loaded() const { return m_buffer_loaded; }
    inline bool is_dirty() const { return m_isDirty; }
    inline void set_dirty(bool d) { m_isDirty = d; }

    inline TextureSettings get_settings() const { return m_settings; }
    inline void set_settings(TextureSettings settings) { m_settings = settings; }

    inline Extent3D get_size() const { return m_image.extent; }
    inline int get_num_channels() const { return m_channels; }
    inline Image get_image() const { return m_image; }
    inline void set_image(Image img) { m_image = img; }

    inline void set_use_mipmaps(bool op) { m_settings.useMipmaps = op; }
    inline void set_anysotropic_filtering(bool op) { m_settings.anisotropicFilter = op; }
    inline void set_format(ColorFormatType f) { m_settings.format = f; }
    inline void set_filter(TextureFilterType f) { m_settings.filter = f; }
    inline void set_adress_mode(TextureAdressModeType am) { m_settings.adressMode = am; }

    void load_image(std::string fileName, bool asyncCall = true);
};
VULKAN_ENGINE_NAMESPACE_END

#endif