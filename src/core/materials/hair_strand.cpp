#include "engine/core/materials/hair_strand.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core
{

Graphics::MaterialUniforms HairStrandMaterial::get_uniforms() const
{
    // Alignment in shader
    //-----------------
    // vec3 baseColor;
    // float thickness;

    // float Rpower;
    // float TTpower;
    // float TRTpower;
    // float roughness;

    // float scatter;
    // float shift;
    // float ior;
    // bool glints;

    // bool useScatter;
    // bool coloredScatter;
    // bool r;
    // bool tt;

    // bool trt;
    // bool occlusion;

    //-----------------

    Graphics::MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_baseColor;
    uniforms.dataSlot1.w = m_thickness;
    uniforms.dataSlot2 = {m_Rpower, m_TTpower, m_TRTpower, m_roughness};
    uniforms.dataSlot3 = {m_scatter, m_shift, m_ior, m_glints};
    uniforms.dataSlot4 = {m_useScatter, m_coloredScatter, m_R, m_TT};
    uniforms.dataSlot5 = {m_TRT, m_occlusion, 0.0f, 0.0f};

    return uniforms;
}
Graphics::MaterialUniforms HairStrandMaterial2::get_uniforms() const
{
    // Alignment in shader
    //-----------------
    // vec3 baseColor;
    // float thickness;

    // float Rpower;
    // float TTpower;
    // float TRTpower;
    // float scatter;

    // bool glints;
    // bool useScatter;
    // bool coloredScatter;
    // bool r;

    // bool tt;
    // bool trt;
    // bool occlusion;

    //-----------------

    Graphics::MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_baseColor;
    uniforms.dataSlot1.w = m_thickness;
    uniforms.dataSlot2 = {m_Rpower, m_TTpower, m_TRTpower, m_scatter};
    uniforms.dataSlot3 = {m_glints, m_useScatter, m_coloredScatter, m_R};
    uniforms.dataSlot4 = {m_TT, m_TRT, m_occlusion, 0.0f};
    return uniforms;
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END
