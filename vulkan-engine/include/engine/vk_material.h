#ifndef VK_MATERIAL
#define VK_MATERIAL

#include <unordered_map>
#include <engine/vk_texture.h>
#include "../private/vk_pipeline.h"
#include "../private/vk_uniforms.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

struct MaterialSettings
{
    bool blending{true};
    bool faceCulling{false};
    CullingMode culling{_BACK};
    bool depthTest{true};
    bool depthWrite{true};
};

class Material
{
protected:
    ShaderPass *m_shaderPass{nullptr};

    MaterialSettings m_settings{};

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
    Material(std::string shaderPassID, MaterialSettings params) : m_shaderPassID(shaderPassID), m_settings(params) {}

    ~Material() {}

    virtual std::string get_shaderpass_id() const { return m_shaderPassID; }
    virtual inline MaterialSettings get_parameters() const { return m_settings; }
    virtual void set_parameters(MaterialSettings p) { m_settings = p; }

    virtual inline void set_enable_culling(bool op) { m_settings.faceCulling = op; }
    virtual inline void set_culling_type(CullingMode t) { m_settings.culling = t; }
    virtual inline void enable_depth_test(bool op) { m_settings.depthTest = op; }
    virtual inline void enable_depth_writes(bool op) { m_settings.depthWrite = op; }
};

VULKAN_ENGINE_NAMESPACE_END
#endif