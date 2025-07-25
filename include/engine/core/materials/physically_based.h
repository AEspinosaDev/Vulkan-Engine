/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PBR_H
#define PBR_H

#include <engine/core/materials/material.h>
#include <engine/graphics/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

/// Epic's Unreal Engine 4 PBR Metallic-Roughness Workflow
class PhysicalMaterial : public IMaterial
{
protected:
    Vec2 m_tileUV = { 1.0f, 1.0f };

    Vec4  m_albedo        = { 0.5, 0.5, 0.5, 1.0 }; // w for opacity
    float m_albedoWeight  = 1.0f;                   // Weight between parameter and albedo texture
    float m_opacityWeight = 1.0f;

    float m_metalness       = 0.0f;
    float m_metalnessWeight = 1.0f; // Weight between parameter and metallness texture

    float m_roughness       = 0.75f;
    float m_roughnessWeight = 1.0f; // Weight between parameter and roughness texture

    float m_occlusion       = 1.0f;
    float m_occlusionWeight = 1.0f; // Weight between parameter and occlusion texture

    Vec3  m_emissionColor     = { 0.0f, 0.0f, 0.0f };
    float m_emisionWeight     = 1.0f; // Weight between parameter and emi texture
    float m_emissionIntensity = 1.0f;

    // Whether to if casts SSR
    bool m_isReflective = false;

    // Query
    bool m_hasAlbedoTexture    = false;
    bool m_hasNormalTexture    = false;
    bool m_useNormalTexture    = true;
    bool m_hasRoughnessTexture = false;
    bool m_hasMetallicTexture  = false;
    bool m_hasAOTexture        = false;
    bool m_hasEmissiveTexture  = false;
    bool m_hasMaskTexture      = false;
    int  m_maskType            = -1;

    enum Textures
    {
        ALBEDO         = 0,
        NORMAL         = 1,
        MASK_ROUGHNESS = 2,
        METALNESS      = 3,
        AO             = 4,
        EMISSIVE       = 5,
    };

    std::unordered_map<int, ITexture*> m_textures { { ALBEDO, nullptr },
                                                    { NORMAL, nullptr },
                                                    { MASK_ROUGHNESS, nullptr },
                                                    { METALNESS, nullptr },
                                                    { AO, nullptr },
                                                    { EMISSIVE, nullptr } };

    std::unordered_map<int, bool> m_textureBindingState;

    virtual std::unordered_map<int, bool> get_texture_binding_state() const {
        return m_textureBindingState;
    }
    virtual void set_texture_binding_state( int id, bool state ) {
        m_textureBindingState[id] = state;
    }

public:
    virtual IMaterial::GPUPayload                     get_uniforms() const;
    virtual inline std::unordered_map<int, ITexture*> get_textures() const {
        return m_textures;
    }
    PhysicalMaterial( Vec4 albedo = Vec4( 1.0f, 1.0f, 0.5f, 1.0f ) )
        : IMaterial( "physical" )
        , m_albedo( albedo ) {
    }
    PhysicalMaterial( Vec4 albedo, MaterialSettings params )
        : IMaterial( "physical", params )
        , m_albedo( albedo ) {
    }

    inline Vec2 get_tile() const {
        return m_tileUV;
    }
    inline void set_tile( Vec2 tile ) {
        m_tileUV  = tile;
        m_isDirty = true;
    }

    inline Vec3 get_albedo() const {
        return Vec3( m_albedo );
    }
    inline void set_albedo( Vec3 c ) {
        m_albedo  = Vec4( c, m_albedo.w );
        m_isDirty = true;
    }

    // Weight between parameter and albedo texture
    virtual inline float get_albedo_weight() const {
        return m_albedoWeight;
    }
    // Weight between parameter and albedo texture
    virtual inline void set_albedo_weight( float w ) {
        m_albedoWeight = w;
        m_isDirty      = true;
    }

    inline float get_opacity() const {
        return m_albedo.a;
    }
    inline void set_opacity( float op ) {
        m_albedo.a = op;
        m_isDirty  = true;
    }
    // Weight between parameter and op texture
    virtual inline float get_opacity_weight() const {
        return m_opacityWeight;
    }
    // Weight between parameter and op texture
    virtual inline void set_opacity_weight( float w ) {
        m_opacityWeight = w;
        m_isDirty       = true;
    }
    inline bool reflective() const {
        return m_isReflective;
    }
    inline void reflective( bool op ) {
        m_isReflective = op;
    }

    inline float get_metalness() const {
        return m_metalness;
    }
    inline void set_metalness( float m ) {
        m_metalness = m;
        m_isDirty   = true;
    }

    // Weight between parameter and metallness texture
    virtual inline float get_metalness_weight() const {
        return m_metalnessWeight;
    }
    // Weight between parameter and metallness texture
    virtual inline void set_metalness_weight( float w ) {
        m_metalnessWeight = w;
        m_isDirty         = true;
    }

    inline float get_roughness() const {
        return m_roughness;
    }
    inline void set_roughness( float r ) {
        m_roughness = r;
        m_isDirty   = true;
    }

    // Weight between parameter and roughness texture
    virtual inline float get_roughness_weight() const {
        return m_roughnessWeight;
    }
    // Weight between parameter and roughness texture
    virtual inline void set_roughness_weight( float w ) {
        m_roughnessWeight = w;
        m_isDirty         = true;
    }

    inline float get_occlusion() const {
        return m_occlusion;
    }
    inline void set_occlusion( float r ) {
        m_occlusion = r;
        m_isDirty   = true;
    }

    // Weight between parameter and occlusion texture
    virtual inline float get_occlusion_weight() const {
        return m_occlusionWeight;
    }
    // Weight between parameter and occlusion texture
    virtual inline void set_occlusion_weight( float w ) {
        m_occlusionWeight = w;
        m_isDirty         = true;
    }

    inline ITexture* get_albedo_texture() {
        return m_textures[ALBEDO];
    }
    inline void set_albedo_texture( ITexture* t ) {
        m_hasAlbedoTexture            = t ? true : false;
        m_textureBindingState[ALBEDO] = false;
        m_textures[ALBEDO]            = t;
        m_isDirty                     = true;
    }

    inline ITexture* get_normal_texture() {
        return m_textures[NORMAL];
    }
    inline void set_normal_texture( ITexture* t ) {
        m_hasNormalTexture            = t ? true : false;
        m_textureBindingState[NORMAL] = false;
        m_textures[NORMAL]            = t;
        m_isDirty                     = true;
    }

    /*
    Sets mask texture. Support for some presets of commercial game engines.
    */
    inline ITexture* get_mask_texture() {
        return m_textures[MASK_ROUGHNESS];
    }
    inline void set_mask_texture( ITexture* t, MaskType preset ) {
        m_hasMaskTexture                      = t ? true : false;
        m_textureBindingState[MASK_ROUGHNESS] = false;
        m_textures[MASK_ROUGHNESS]            = t;
        m_maskType                            = (int)preset;
        m_isDirty                             = true;
    }

    inline ITexture* get_roughness_texture() {
        return m_textures[MASK_ROUGHNESS];
    }
    inline void set_roughness_texture( ITexture* t ) {
        m_hasRoughnessTexture                 = t ? true : false;
        m_textureBindingState[MASK_ROUGHNESS] = false;
        m_textures[MASK_ROUGHNESS]            = t;
        m_isDirty                             = true;
    }
    inline ITexture* get_metallic_texture() {
        return m_textures[METALNESS];
    }
    inline void set_metallic_texture( ITexture* t ) {
        m_hasMetallicTexture             = t ? true : false;
        m_textureBindingState[METALNESS] = false;
        m_textures[METALNESS]            = t;
        m_isDirty                        = true;
    }
    inline ITexture* get_occlusion_texture() {
        return m_textures[AO];
    }
    inline void set_occlusion_texture( ITexture* t ) {
        m_hasAOTexture            = t ? true : false;
        m_textureBindingState[AO] = false;
        m_textures[AO]            = t;
        m_isDirty                 = true;
    }
    inline float get_emissive_weight() const {
        return m_emisionWeight;
    };
    inline void set_emissive_weight( float w ) {
        m_emisionWeight = w;
    };
    inline float get_emission_intensity() const {
        return m_emissionIntensity;
    };
    inline void set_emission_intensity( float i ) {
        m_emissionIntensity = i;
    };
    inline Vec3 get_emissive_color() const {
        return m_emissionColor;
    };
    inline void set_emissive_color( Vec3 w ) {
        m_emissionColor = w;
    };
    inline ITexture* get_emissive_texture() {
        return m_textures[EMISSIVE];
    }
    inline void set_emissive_texture( ITexture* t ) {
        m_hasEmissiveTexture            = t ? true : false;
        m_textureBindingState[EMISSIVE] = false;
        m_textures[EMISSIVE]            = t;
        m_isDirty                       = true;
    }
    inline MaskType get_mask_type() const {
        return (MaskType)m_maskType;
    }
    inline void use_normal_texture( bool op ) {
        m_useNormalTexture = op;
    }
    inline bool use_normal_texture() const {
        return m_useNormalTexture;
    }
};
} // namespace Core
VULKAN_ENGINE_NAMESPACE_END
#endif