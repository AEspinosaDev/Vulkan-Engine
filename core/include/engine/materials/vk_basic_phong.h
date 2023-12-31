#ifndef VK_BASIC_PHONG_MATERIAL
#define VK_BASIC_PHONG_MATERIAL

#include "../vk_material.h"
#include "../private/vk_descriptors.h"


namespace vke
{
    class BasicPhongMaterial : public Material
    {
    protected:
        glm::vec4 m_color; // w for transparency
        enum Textures
        {
            ALBEDO = 0,
            GLOSSINESS = 1,
            NORMAL = 2,
        };
        std::unordered_map<int, Texture *> m_textures;
        

        virtual void upload_uniforms();
        virtual inline std::unordered_map<int, Texture *> get_textures() const
        {
            return m_textures;
        }

    public:
        BasicPhongMaterial(glm::vec4 color = glm::vec4(1.0, 1.0, 0.5, 1.0)) : Material("basic_phong"), m_color(color) {}

        inline void set_color(glm::vec4 c) { m_color = c; }
        inline void set_color_texture(Texture *t)
        {
            m_textures[ALBEDO] = t;
        }
        inline Texture *get_color_texture()  { return m_textures[ALBEDO]; }
        inline glm::vec4 get_color() const { return m_color; }
    };
}
#endif