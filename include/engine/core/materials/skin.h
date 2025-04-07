/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SKIN_H
#define SKIN_H

#include <engine/core/materials/material.h>
#include <engine/graphics/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/// SSSS
/*
///////////////////
W I P

///////////////////
*/
class SkinMaterial : public IMaterial
{
  protected:
    Vec2 m_tileUV = {1.0f, 1.0f};

    Vec4  m_albedo       = {0.5, 0.5, 0.5, 1.0}; // w for opacity
    float m_albedoWeight = 1.0f;                 // Weight between parameter and albedo texture

    float m_roughness       = 0.75f;
    float m_roughnessWeight = 1.0f; // Weight between parameter and roughness texture

    float m_occlusion       = 1.0f;
    float m_occlusionWeight = 1.0f; // Weight between parameter and occlusion texture

    bool m_isReflective  = false;
    bool m_temporalReuse = false;

    // Query
    bool m_hasAlbedoTexture    = false;
    bool m_hasNormalTexture    = false;
    bool m_useNormalTexture    = true;
    bool m_hasRoughnessTexture = false;
    bool m_hasAOTexture        = false;
    bool m_hasMaskTexture      = false;

    enum Textures
    {
        ALBEDO    = 0,
        NORMAL    = 1,
        ROUGHNESS = 2,
        AO        = 3,
        MASK      = 4,
    };

    std::unordered_map<int, ITexture*> m_textures{{ALBEDO, nullptr},
                                                  {NORMAL, nullptr},
                                                  {ROUGHNESS, nullptr},
                                                  {AO, nullptr},
                                                  {MASK, nullptr}};

    std::unordered_map<int, bool> m_textureBindingState;

    virtual std::unordered_map<int, bool> get_texture_binding_state() const {
        return m_textureBindingState;
    }
    virtual void set_texture_binding_state(int id, bool state) {
        m_textureBindingState[id] = state;
    }

  public:
    virtual Graphics::MaterialUniforms                get_uniforms() const;
    virtual inline std::unordered_map<int, ITexture*> get_textures() const {
        return m_textures;
    }
    SkinMaterial(Vec4 albedo = Vec4(1.0f, 1.0f, 0.5f, 1.0f))
        : IMaterial("skin")
        , m_albedo(albedo) {
    }
    SkinMaterial(Vec4 albedo, MaterialSettings params)
        : IMaterial("skin", params)
        , m_albedo(albedo) {
    }

    inline Vec2 get_tile() const {
        return m_tileUV;
    }
    inline void set_tile(Vec2 tile) {
        m_tileUV  = tile;
        m_isDirty = true;
    }

    inline Vec3 get_albedo() const {
        return Vec3(m_albedo);
    }
    inline void set_albedo(Vec3 c) {
        m_albedo  = Vec4(c, m_albedo.w);
        m_isDirty = true;
    }

    // Weight between parameter and albedo texture
    virtual inline float get_albedo_weight() const {
        return m_albedoWeight;
    }
    // Weight between parameter and albedo texture
    virtual inline void set_albedo_weight(float w) {
        m_albedoWeight = w;
        m_isDirty      = true;
    }

    inline float get_opacity() const {
        return m_albedo.a;
    }
    inline void set_opacity(float op) {
        m_albedo.a = op;
        m_isDirty  = true;
    }
    inline bool reflective() const {
        return m_isReflective;
    }
    inline void reflective(bool op) {
        m_isReflective = op;
    }
    inline float get_roughness() const {
        return m_roughness;
    }
    inline void set_roughness(float r) {
        m_roughness = r;
        m_isDirty   = true;
    }
    // Weight between parameter and roughness texture
    virtual inline float get_roughness_weight() const {
        return m_roughnessWeight;
    }
    // Weight between parameter and roughness texture
    virtual inline void set_roughness_weight(float w) {
        m_roughnessWeight = w;
        m_isDirty         = true;
    }

    inline float get_occlusion() const {
        return m_occlusion;
    }
    inline void set_occlusion(float r) {
        m_occlusion = r;
        m_isDirty   = true;
    }

    // Weight between parameter and occlusion texture
    virtual inline float get_occlusion_weight() const {
        return m_occlusionWeight;
    }
    // Weight between parameter and occlusion texture
    virtual inline void set_occlusion_weight(float w) {
        m_occlusionWeight = w;
        m_isDirty         = true;
    }

    inline ITexture* get_albedo_texture() {
        return m_textures[ALBEDO];
    }
    inline void set_albedo_texture(ITexture* t) {
        m_hasAlbedoTexture            = t ? true : false;
        m_textureBindingState[ALBEDO] = false;
        m_textures[ALBEDO]            = t;
        m_isDirty                     = true;
    }

    inline ITexture* get_normal_texture() {
        return m_textures[NORMAL];
    }
    inline void set_normal_texture(ITexture* t) {
        m_hasNormalTexture            = t ? true : false;
        m_textureBindingState[NORMAL] = false;
        m_textures[NORMAL]            = t;
        m_isDirty                     = true;
    }

    /*
    Sets mask texture. Support for some presets of commercial game engines.
    */
    inline ITexture* get_mask_texture() {
        return m_textures[MASK];
    }
    inline void set_mask_texture(ITexture* t) {
        m_hasMaskTexture            = t ? true : false;
        m_textureBindingState[MASK] = false;
        m_textures[MASK]            = t;
        m_isDirty                   = true;
    }

    inline ITexture* get_roughness_texture() {
        return m_textures[ROUGHNESS];
    }
    inline void set_roughness_texture(ITexture* t) {
        m_hasRoughnessTexture            = t ? true : false;
        m_textureBindingState[ROUGHNESS] = false;
        m_textures[ROUGHNESS]            = t;
        m_isDirty                        = true;
    }
    inline ITexture* get_occlusion_texture() {
        return m_textures[AO];
    }
    inline void set_occlusion_texture(ITexture* t) {
        m_hasAOTexture            = t ? true : false;
        m_textureBindingState[AO] = false;
        m_textures[AO]            = t;
        m_isDirty                 = true;
    }
    inline void use_normal_texture(bool op) {
        m_useNormalTexture = op;
    }
    inline bool use_normal_texture() const {
        return m_useNormalTexture;
    }
    inline void use_temporal_reuse(bool op) {
        m_temporalReuse = op;
    }
    inline bool use_temporal_reuse() const {
        return m_temporalReuse;
    }
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
#endif