/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef HAIR_H
#define HAIR_H

#include <engine/core/materials/material.h>
#include <engine/graphics/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

/// Epic's Fitted Marschner Workflow. Only works with geometry defined as lines.
class HairMaterial : public IMaterial
{
  protected:
    Vec4 m_baseColor{0.27f, 0.14f, 0.04f, 1.0f}; // w for opacity

    float m_thickness{0.003f};

    bool m_R{true}; // Reflection
    float m_Rpower{5.0f};

    bool m_TT{true}; // Transmitance
    float m_TTpower{1.0f};

    bool m_TRT{true}; // Second reflection
    float m_TRTpower{15.0f};

    float m_roughness{0.4f};
    float m_scatter{500.0f};
    float m_shift{0.12f}; // In radians (-5º to -10º) => 0.088 to 0.17 //Not with epic 0.02 does fine
    float m_ior{1.55f};

    bool m_glints{false};
    bool m_useScatter{false};
    bool m_coloredScatter{false};
    bool m_occlusion{false};

    std::unordered_map<int, Texture *> m_textures;

    std::unordered_map<int, bool> m_textureBindingState;

    virtual Graphics::MaterialUniforms get_uniforms() const;
    virtual inline std::unordered_map<int, Texture *> get_textures() const
    {
        return m_textures;
    }

    virtual std::unordered_map<int, bool> get_texture_binding_state() const
    {
        return m_textureBindingState;
    }
    virtual void set_texture_binding_state(int id, bool state)
    {
        m_textureBindingState[id] = state;
    }

  public:
    HairMaterial(Vec4 baseColor = Vec4(1.0f, 1.0f, 0.5f, 1.0f)) : IMaterial("hair"), m_baseColor(baseColor)
    {
    }
    HairMaterial(Vec4 baseColor, MaterialSettings params) : IMaterial("hair", params), m_baseColor(baseColor)
    {
    }

    inline Vec3 get_base_color() const
    {
        return Vec3(m_baseColor);
    }
    inline void set_base_color(Vec3 c)
    {
        m_baseColor = Vec4(c, m_baseColor.w);
        m_isDirty = true;
    }

    float get_thickness() const
    {
        return m_thickness;
    }
    void set_thickness(float thickness)
    {
        m_thickness = thickness;
    }

    // Primary reflection toggle
    bool get_R() const
    {
        return m_R;
    }
    void set_R(bool R)
    {
        m_R = R;
        m_isDirty = true;
    }

    // Primary reflection scale
    float get_Rpower() const
    {
        return m_Rpower;
    }
    void set_Rpower(float Rpower)
    {
        m_Rpower = Rpower;
        m_isDirty = true;
    }

    // Transmitance reflection toggle
    bool get_TT() const
    {
        return m_TT;
    }
    void set_TT(bool TT)
    {
        m_TT = TT;
    }

    // Transmitance reflection scale
    float get_TTpower() const
    {
        return m_TTpower;
    }
    void set_TTpower(float TTpower)
    {
        m_TTpower = TTpower;
        m_isDirty = true;
    }

    // Secoundary reflection toggle
    bool get_TRT() const
    {
        return m_TRT;
    }
    void set_TRT(bool TRT)
    {
        m_TRT = TRT;
    }

    // Secoundary reflection scale
    float get_TRTpower() const
    {
        return m_TRTpower;
    }
    void set_TRTpower(float TRTpower)
    {
        m_TRTpower = TRTpower;
        m_isDirty = true;
    }

    float get_roughness() const
    {
        return m_roughness;
    }
    void set_roughness(float roughness)
    {
        m_roughness = roughness;
        m_isDirty = true;
    }

    float get_scatter() const
    {
        return m_scatter;
    }
    void set_scatter(float scatter)
    {
        m_scatter = scatter;
        m_isDirty = true;
    }

    float get_shift() const
    {
        return m_shift;
    }
    void set_shift(float shift)
    {
        m_shift = shift;
        m_isDirty = true;
    }

    float get_ior() const
    {
        return m_ior;
    }
    void set_ior(float ior)
    {
        m_ior = ior;
        m_isDirty = true;
    }

    bool get_glints() const
    {
        return m_glints;
    }
    void set_glints(bool glints)
    {
        m_glints = glints;
        m_isDirty = true;
    }

    bool get_useScatter() const
    {
        return m_useScatter;
    }
    void set_useScatter(bool useScatter)
    {
        m_useScatter = useScatter;
        m_isDirty = true;
    }

    bool get_coloredScatter() const
    {
        return m_coloredScatter;
    }
    void set_coloredScatter(bool coloredScatter)
    {
        m_coloredScatter = coloredScatter;
        m_isDirty = true;
    }

    bool get_occlusion() const
    {
        return m_occlusion;
    }
    void set_occlusion(bool occlusion)
    {
        m_occlusion = occlusion;
        m_isDirty = true;
    }
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
#endif