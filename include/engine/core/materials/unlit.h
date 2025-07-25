/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BASIC_UNLIT_H
#define BASIC_UNLIT_H

#include <engine/core/materials/material.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class UnlitMaterial : public IMaterial
{
  protected:
    glm::vec2 m_tileUV{1.0f, 1.0f};

    glm::vec4 m_color; // w for opacity

    bool m_hasColorTexture{false};

    enum Textures
    {
        COLOR = 0,
    };

    std::unordered_map<int, ITexture*> m_textures{{COLOR, nullptr},
                                                  {1, nullptr},
                                                  {2, nullptr},
                                                  {3, nullptr},
                                                  {4, nullptr},
                                                  {5, nullptr}};

    std::unordered_map<int, bool> m_textureBindingState;

    virtual IMaterial::GPUPayload                 get_uniforms() const;
    virtual inline std::unordered_map<int, ITexture*> get_textures() const {
        return m_textures;
    }

    virtual std::unordered_map<int, bool> get_texture_binding_state() const {
        return m_textureBindingState;
    }
    virtual void set_texture_binding_state(int id, bool state) {
        m_textureBindingState[id] = state;
    }

  public:
    UnlitMaterial(glm::vec4 color = glm::vec4(1.0, 1.0, 0.5, 1.0))
        : IMaterial("unlit")
        , m_color(color) {
    }
    UnlitMaterial(glm::vec4 color, MaterialSettings params)
        : IMaterial("unlit", params)
        , m_color(color) {
    }

    inline glm::vec2 get_tile() const {
        return m_tileUV;
    }
    inline void set_tile(glm::vec2 tile) {
        m_tileUV  = tile;
        m_isDirty = true;
    }

    inline void set_color(glm::vec4 c) {
        m_color   = c;
        m_isDirty = true;
    }
    inline glm::vec4 get_color() const {
        return m_color;
    }

    // Texture must have A channel reserved for OPACITY
    inline void set_color_texture(ITexture* t) {
        m_textures[COLOR] = t;
        m_isDirty         = true;
    }
    inline ITexture* get_color_texture() {
        return m_textures[COLOR];
    }
};
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
#endif