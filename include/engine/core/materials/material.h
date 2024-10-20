/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef MATERIAL_H
#define MATERIAL_H

#include <engine/core/textures/texture.h>
#include <engine/graphics/shaderpass.h>
#include <engine/graphics/uniforms.h>
#include <unordered_map>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

struct MaterialSettings
{
    bool blending{false};
    bool faceCulling{false};
    CullingMode culling{_BACK};
    bool depthTest{true};
    bool depthWrite{true};
    bool alphaTest{false};
};

class Material
{
  protected:
    MaterialSettings m_settings{};

    std::string m_shaderPassID{};

    Graphics::DescriptorSet m_textureDescriptor{};

    bool m_isDirty{true};

    friend class Renderer;

  public:
    static Material *DEBUG_MATERIAL;

    Material(std::string shaderPassID) : m_shaderPassID(shaderPassID)
    {
    }
    Material(std::string shaderPassID, MaterialSettings params) : m_shaderPassID(shaderPassID), m_settings(params)
    {
    }

    ~Material()
    {
    }

    virtual std::string get_shaderpass_ID() const
    {
        return m_shaderPassID;
    }
    virtual inline MaterialSettings get_parameters() const
    {
        return m_settings;
    }
    virtual void set_parameters(MaterialSettings p)
    {
        m_settings = p;
    }

    virtual inline void set_enable_culling(bool op)
    {
        m_settings.faceCulling = op;
    }
    virtual inline void set_culling_type(CullingMode t)
    {
        m_settings.culling = t;
    }
    virtual inline void enable_depth_test(bool op)
    {
        m_settings.depthTest = op;
    }
    virtual inline void enable_depth_writes(bool op)
    {
        m_settings.depthWrite = op;
    }
    virtual inline void enable_alpha_test(bool op)
    {
        m_settings.alphaTest = op;
    }
    virtual inline void enable_blending(bool op)
    {
        m_settings.blending = op;
    }

    virtual inline Graphics::DescriptorSet &get_texture_descriptor()
    {
        return m_textureDescriptor;
    }

    virtual Graphics::MaterialUniforms get_uniforms() const = 0;

    virtual std::unordered_map<int, Texture *> get_textures() const = 0;

    virtual std::unordered_map<int, bool> get_texture_binding_state() const = 0;
    virtual void set_texture_binding_state(int id, bool state) = 0;
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
#endif