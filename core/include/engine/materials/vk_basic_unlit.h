#ifndef VK_BASIC_UNLIT_MATERIAL
#define VK_BASIC_UNLIT_MATERIAL

#include "../vk_material.h"

namespace vke
{
    class BasicUnlitMaterial : public Material
    {
    protected:
        glm::vec4 m_color; // w for transparency

        virtual void upload_uniforms();
        virtual inline  std::unordered_map<int, Texture *> get_textures() const
        {
            return  std::unordered_map<int, Texture *>();
        }

    public:
        BasicUnlitMaterial(glm::vec4 color = glm::vec4(1.0, 1.0, 0.5, 1.0)) : Material("basic_unlit"), m_color(color) {}

        inline void set_color(glm::vec4 c) { m_color = c; }
        inline glm::vec4 get_color() { return m_color; }
    };
}
#endif