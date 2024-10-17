#include "engine/core/materials/hair.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

graphics::MaterialUniforms HairMaterial::get_uniforms() const
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

    graphics::MaterialUniforms uniforms;
    uniforms.dataSlot1 = m_baseColor;
    uniforms.dataSlot1.w = m_thickness;
    uniforms.dataSlot2 = {m_Rpower, m_TTpower, m_TRTpower, m_roughness};
    uniforms.dataSlot3 = {m_scatter, m_shift, m_ior, m_glints};
    uniforms.dataSlot4 = {m_useScatter, m_coloredScatter, m_R, m_TT};
    uniforms.dataSlot5 = {m_TRT, m_occlusion, 0.0f, 0.0f};

    return uniforms;
}

VULKAN_ENGINE_NAMESPACE_END
