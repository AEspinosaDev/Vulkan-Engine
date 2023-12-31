#ifndef VK_BASIC_PHONG_MATERIAL
#define VK_BASIC_PHONG_MATERIAL

#include "../vk_material.h"
#include "../private/vk_descriptors.h"

namespace vke
{
    class BasicPhongMaterial : public Material
    {
    protected:
        glm::vec2 m_tileUV{1.0f, 1.0f};

        glm::vec4 m_color; // w for transparency

        float m_glossiness{40.0f};
        float m_shininess{10.0f};

        bool m_hasColorTexture{false};
        bool m_hasOpacityTexture{false};
        bool m_hasNormalTexture{false};
        bool m_hasGlossinessTexture{false};
        bool m_hasShininessTexture{false};

        enum Textures
        {
            ALBEDO = 0,
            NORMAL = 1,
            GLOSSINESS = 2,
            SHININESS = 3,
            OPACITY = 4
        };
        std::unordered_map<int, Texture *> m_textures;
    

        virtual MaterialUniforms get_uniforms() const;
        virtual inline std::unordered_map<int, Texture *> get_textures() const
        {
            return m_textures;
        }

    public:
        BasicPhongMaterial(glm::vec4 color = glm::vec4(1.0, 1.0, 0.5, 1.0)) : Material("basic_phong"), m_color(color) {}

        inline glm::vec2 get_tile() const { return m_tileUV; }
        inline void set_tile(glm::vec2 tile) { m_tileUV = tile; }

        inline glm::vec4 get_color() const { return m_color; }
        inline void set_color(glm::vec4 c) { m_color = c; }

        inline float get_glossiness() const { return m_glossiness; }
        inline void set_glossiness(float g) { m_glossiness = g; }

        inline float get_shininess() const { return m_shininess; }
        inline void set_shininess(float s) { m_shininess = s; }

        inline Texture *get_color_texture() { return m_textures[ALBEDO]; }
        inline void set_color_texture(Texture *t)
        {
            m_hasColorTexture = t ? true : false;
            m_textures[ALBEDO] = t;
        }

        inline Texture *get_normal_texture() { return m_textures[NORMAL]; }
        inline void set_normal_texture(Texture *t)
        {
            m_hasNormalTexture = t ? true : false;
            m_textures[NORMAL] = t;
        }

        inline Texture *get_glossiness_texture() { return m_textures[GLOSSINESS]; }
        inline void set_glossiness_texture(Texture *t)
        {
            m_hasGlossinessTexture = t ? true : false;
            m_textures[GLOSSINESS] = t;
        }

        inline Texture *get_opacity_texture() { return m_textures[OPACITY]; }
        inline void set_opacity_texture(Texture *t)
        {
            m_hasOpacityTexture = t ? true : false;
            m_textures[OPACITY] = t;
        }
    };
}
#endif