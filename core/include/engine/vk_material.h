#ifndef VK_MATERIAL
#define VK_MATERIAL
#include <unordered_map>
#include "../private/vk_pipeline.h"
#include "../private/vk_uniforms.h"
#include "vk_texture.h"

namespace vke
{
    enum Culling
    {
        _FRONT,
        _BACK
    };
    struct MaterialParameters
    {
        bool blending{true};
        bool faceCulling{false};
        Culling culling{_BACK};
    };

    class Material
    {
    protected:
        ShaderPass *m_shaderPass{nullptr};

        MaterialParameters m_parameters{};

        std::string m_shaderPassID{};

        DescriptorSet m_textureDescriptor{};

        bool m_isDirty{true};

        friend class Renderer;

        virtual MaterialUniforms get_uniforms() const = 0;

        virtual std::unordered_map<int, Texture *> get_textures() const = 0;

        virtual std::unordered_map<int, bool> get_texture_binding_state() const = 0;
        virtual void set_texture_binding_state(int id, bool state) = 0;

    public:
        static Material *DEBUG_MATERIAL;

        Material(std::string shaderPassID) : m_shaderPassID(shaderPassID) {}
        Material(std::string shaderPassID, MaterialParameters params) : m_shaderPassID(shaderPassID), m_parameters(params) {}

        ~Material() {}

        virtual inline MaterialParameters get_parameters() const { return m_parameters; }
        virtual void set_parameters(MaterialParameters p) { m_parameters = p; }
    };
    
}
#endif