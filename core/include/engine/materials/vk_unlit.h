#ifndef VK_BASIC_UNLIT_MATERIAL
#define VK_BASIC_UNLIT_MATERIAL

#include "../vk_material.h"

namespace vke
{
    class UnlitMaterial : public Material
    {
    protected:
        glm::vec2 m_tileUV{1.0f, 1.0f};

        glm::vec4 m_color; // w for opacity

        bool m_hasColorTexture{false};

        std::unordered_map<int, Texture *> m_textures;

        virtual MaterialUniforms get_uniforms() const;
        virtual inline std::unordered_map<int, Texture *> get_textures() const
        {
            return std::unordered_map<int, Texture *>();
        }

    public:
        UnlitMaterial(glm::vec4 color = glm::vec4(1.0, 1.0, 0.5, 1.0)) : Material("unlit"), m_color(color) {}
        UnlitMaterial(glm::vec4 color, MaterialParameters params) : Material("unlit", params), m_color(color) {}

        inline glm::vec2 get_tile() const { return m_tileUV; }
        inline void set_tile(glm::vec2 tile) { m_tileUV = tile; }

        inline void set_color(glm::vec4 c) { m_color = c; }
        inline glm::vec4 get_color() const { return m_color; }

        // Texture must have A channel reserved for OPACITY
        inline void set_color_texture(Texture *t)
        {
            m_textures[0] = t;
        }
        inline Texture *get_color_texture() { return m_textures[0]; }

       
    };
}
#endif