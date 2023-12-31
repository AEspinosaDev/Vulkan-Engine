#ifndef VK_MATERIAL
#define VK_MATERIAL
#include <unordered_map>
#include "../private/vk_pipeline.h"
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
        bool blending;
        bool faceCulling;
        Culling culling;
    };

    class Material
    {
    protected:
        ShaderPass *m_shaderPass{nullptr};

        MaterialParameters m_parameters;

        std::string m_shaderPassID{};

        friend class Renderer;

        virtual void upload_uniforms() = 0;
        virtual std::unordered_map<int,Texture*> get_textures() const = 0;

    public:
        Material(std::string shaderPassID) : m_shaderPassID(shaderPassID) {}

        ~Material() {}

        virtual inline MaterialParameters get_parameters() const { return m_parameters; }
        virtual void set_parameters(MaterialParameters p) { m_parameters = p; }
    };

}

#endif