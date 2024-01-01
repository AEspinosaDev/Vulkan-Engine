#ifndef VK_BASIC_UNLIT_MATERIAL
#define VK_BASIC_UNLIT_MATERIAL

#include "../vk_material.h"

namespace vke
{
    class BasicUnlitMaterial : public Material
    {
    protected:
        glm::vec2 m_tileUV{1.0f, 1.0f};

        glm::vec4 m_color; // w for opacity

        bool m_hasColorTexture{false};
        bool m_hasOpacityTexture{false};

        enum Textures
        {
            COLOR = 0,
            OPACITY = 1,
        };
        std::unordered_map<int, Texture *> m_textures;

        virtual MaterialUniforms get_uniforms() const;
        virtual inline std::unordered_map<int, Texture *> get_textures() const
        {
            return std::unordered_map<int, Texture *>();
        }

    public:
        BasicUnlitMaterial(glm::vec4 color = glm::vec4(1.0, 1.0, 0.5, 1.0)) : Material("basic_unlit"), m_color(color) {}

        inline glm::vec2 get_tile() const { return m_tileUV; }
        inline void set_tile(glm::vec2 tile) { m_tileUV = tile; }

        inline void set_color(glm::vec4 c) { m_color = c; }
        inline glm::vec4 get_color() const { return m_color; }

        inline void set_color_texture(Texture *t)
        {
            m_textures[COLOR] = t;
        }
        inline Texture *get_color_texture() { return m_textures[COLOR]; }
        
        inline void set_opacity_texture(Texture *t)
        {
            m_textures[OPACITY] = t;
        }
        inline Texture *get_opacity_texture() { return m_textures[OPACITY]; }
    };
}
#endif