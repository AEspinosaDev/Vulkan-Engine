/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BASIC_UNLIT_H
#define BASIC_UNLIT_H

#include <engine/core/material.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class UnlitMaterial : public Material
{
protected:
    glm::vec2 m_tileUV{1.0f, 1.0f};

    glm::vec4 m_color; // w for opacity

    bool m_hasColorTexture{false};

    std::unordered_map<int, Texture *> m_textures;
    std::unordered_map<int, bool> m_textureBindingState;

    virtual MaterialUniforms get_uniforms() const;
    virtual inline std::unordered_map<int, Texture *> get_textures() const
    {
        return m_textures;
    }

    virtual std::unordered_map<int, bool> get_texture_binding_state() const
    {
        return m_textureBindingState;
    }
    virtual void set_texture_binding_state(int id, bool state)
    {
        m_textureBindingState[id] = state;
    }

public:
    UnlitMaterial(glm::vec4 color = glm::vec4(1.0, 1.0, 0.5, 1.0)) : Material("unlit"), m_color(color) {}
    UnlitMaterial(glm::vec4 color, MaterialSettings params) : Material("unlit", params), m_color(color) {}

    inline glm::vec2 get_tile() const { return m_tileUV; }
    inline void set_tile(glm::vec2 tile)
    {
        m_tileUV = tile;
        m_isDirty = true;
    }

    inline void set_color(glm::vec4 c)
    {
        m_color = c;
        m_isDirty = true;
    }
    inline glm::vec4 get_color() const
    {
        return m_color;
    }

    // Texture must have A channel reserved for OPACITY
    inline void set_color_texture(Texture *t)
    {
        m_textures[0] = t;
        m_isDirty = true;
    }
    inline Texture *get_color_texture() { return m_textures[0]; }
};

VULKAN_ENGINE_NAMESPACE_END
#endif