#ifndef VK_PBR_MATERIAL
#define VK_PBR_MATERIAL

#include "../vk_material.h"
#include "../private/vk_descriptors.h"

namespace vke
{
    /// Epic's Unreal Engine 4 PBR Metallic-Roughness Workflow
    class PhysicalBasedMaterial : public Material
    {
    protected:
        glm::vec2 m_tileUV{1.0f, 1.0f};

        glm::vec4 m_albedo;         // w for opacity
        float m_albedoWeight{1.0f}; // Weight between parameter and albedo texture

        float m_metalness{0.5f};
        float m_metalnessWeight{1.0f}; // Weight between parameter and metallness texture

        float m_roughness{0.5f};
        float m_roughnessWeight{1.0f}; // Weight between parameter and roughness texture

        float m_emmissive{0.0f};
        glm::vec4 m_emissionColor;

        bool m_hasAlbedoTexture{false};
        bool m_hasNormalTexture{false};
        bool m_hasMaskTexture{false};

        enum Textures
        {
            ALBEDO = 0,
            NORMAL = 1,
            MASK = 2, // R = ROUGHNESS // G = METALNESS // B = AO
        };

        std::unordered_map<int, Texture *> m_textures;

        virtual MaterialUniforms get_uniforms() const;
        virtual inline std::unordered_map<int, Texture *> get_textures() const
        {
            return m_textures;
        }

    public:
        PhysicalBasedMaterial(glm::vec4 albedo = glm::vec4(1.0f, 1.0f, 0.5f, 1.0f)) : Material("physical"), m_albedo(albedo) {}
        PhysicalBasedMaterial(glm::vec4 albedo, MaterialParameters params) : Material("physical", params), m_albedo(albedo) {}

        inline glm::vec2 get_tile() const { return m_tileUV; }
        inline void set_tile(glm::vec2 tile) { m_tileUV = tile; }

        inline glm::vec4 get_albedo() const { return m_albedo; }
        inline void set_albedo(glm::vec4 c) { m_albedo = c; }

        // Weight between parameter and albedo texture
        virtual inline float get_albedo_weight() const { return m_albedoWeight; }
        // Weight between parameter and albedo texture
        virtual inline void set_albedo_weight(float w) { m_albedoWeight = w; }

        inline float get_metalness() const { return m_metalness; }
        inline void set_metalness(float m) { m_metalness = m; }

        // Weight between parameter and metallness texture
        virtual inline float get_metalness_weight() const { return m_metalnessWeight; }
        // Weight between parameter and metallness texture
        virtual inline void set_metalness_weight(float w) { m_metalnessWeight = w; }

        inline float get_roughness() const { return m_roughness; }
        inline void set_roughness(float r) { m_roughness = r; }

        // Weight between parameter and roughness texture
        virtual inline float get_roughness_weight() const { return m_roughnessWeight; }
        // Weight between parameter and roughness texture
        virtual inline void set_roughness_weight(float w) { m_roughnessWeight = w; }

        inline Texture *get_albedo_texture() { return m_textures[ALBEDO]; }
        inline void set_albedo_texture(Texture *t)
        {
            m_hasAlbedoTexture = t ? true : false;
            m_textures[ALBEDO] = t;
        }

        inline Texture *get_normal_texture() { return m_textures[NORMAL]; }
        inline void set_normal_texture(Texture *t)
        {
            m_hasNormalTexture = t ? true : false;
            m_textures[NORMAL] = t;
        }

        /*
        Sets mask texture. Mask texture channels must represent: R = ROUGHNESS // G = METALNESS // B = AO
        */
        inline Texture *get_mask_texture() { return m_textures[MASK]; }
        inline void set_mask_texture(Texture *t)
        {
            m_hasMaskTexture = t ? true : false;
            m_textures[MASK] = t;
        }
    };
}
#endif