/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef LIGHT_H
#define LIGHT_H

#include <engine/core/scene/object3D.h>
#include <engine/graphics/uniforms.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class Light : public Object3D
{
  protected:
    Vec3  m_color;
    float m_intensity;
    float m_area = 0.0; // If ZERO is treated as a VPL (Virtual Point Light)

    // Shadow
    struct Shadow {
        ShadowType type                = ShadowType::BASIC_SHADOW;
        bool       cast                = true;
        float      nearPlane           = .5f;
        float      farPlane            = 96.0f;
        float      fov                 = 45.0f;
        Vec3       target              = {0.0f, 0.0f, 0.0f};
        float      bias                = 0.005f;
        float      bleeding            = 0.75f;
        bool       angleDependableBias = false;
        float      kernelRadius        = 1.0;
        int        softness            = 3;
        int        raySamples          = 4;
    };

    Shadow          m_shadow;
    const LightType m_lighType;

    static uint16_t m_nonRaytraceCount;

  public:
    Light(std::string name, LightType type, Vec3 color = Vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f)
        : Object3D(name, LIGHT)
        , m_color(color)
        , m_intensity(intensity)
        , m_lighType(type) {
        if (m_shadow.type != ShadowType::RAYTRACED_SHADOW)
            m_nonRaytraceCount++;
    }

    virtual inline Vec3 get_color() const {
        return m_color;
    }
    virtual inline void set_color(Vec3 c) {
        m_color = c;
    }

    virtual inline float get_intensity() const {
        return m_intensity;
    }
    virtual inline void set_intensity(float i) {
        m_intensity = i;
    }

    virtual inline bool get_cast_shadows() const {
        return m_shadow.cast;
    }
    virtual inline void set_cast_shadows(bool o) {
        m_shadow.cast = o;
    }
    virtual inline ShadowType get_shadow_type() const {
        return m_shadow.type;
    }
    virtual inline void set_shadow_type(ShadowType t) {
        auto oldType  = m_shadow.type;
        m_shadow.type = t;
        if (m_shadow.type != ShadowType::RAYTRACED_SHADOW && oldType == ShadowType::RAYTRACED_SHADOW)
        {
            m_nonRaytraceCount++;
        } else if (oldType != ShadowType::RAYTRACED_SHADOW)
        { m_nonRaytraceCount--; }
    }
    virtual inline Vec3 get_shadow_target() const {
        return m_shadow.target;
    }
    virtual inline void set_shadow_target(Vec3 o) {
        m_shadow.target = o;
    }

    virtual inline float get_shadow_near() const {
        return m_shadow.nearPlane;
    }
    virtual inline void set_shadow_near(float n) {
        m_shadow.nearPlane = n;
    }

    virtual inline float get_shadow_far() const {
        return m_shadow.farPlane;
    }
    virtual inline void set_shadow_far(float f) {
        m_shadow.farPlane = f;
    }

    virtual inline float get_shadow_fov() const {
        return m_shadow.fov;
    }
    virtual inline void set_shadow_fov(float f) {
        m_shadow.fov = f;
    }

    virtual inline float get_shadow_bias() const {
        return m_shadow.bias;
    }
    virtual inline void set_shadow_bias(float b) {
        m_shadow.bias = b;
    }
    virtual inline float get_shadow_bleeding() const {
        return m_shadow.bleeding;
    }
    virtual inline void set_shadow_bleeding(float b) {
        m_shadow.bleeding = b;
    }

    virtual inline int get_shadow_softness() const {
        return m_shadow.softness;
    }
    virtual inline void set_shadow_softness(int k) {
        m_shadow.softness = k;
    }
    virtual inline int get_shadow_ray_samples() const {
        return m_shadow.raySamples;
    }
    virtual inline void set_shadow_ray_samples(int rays) {
        m_shadow.raySamples = rays;
    }

    virtual inline bool get_angle_dependant_bias() const {
        return m_shadow.angleDependableBias;
    }
    virtual inline void set_angle_dependant_bias(bool o) {
        m_shadow.angleDependableBias = o;
    }

    virtual inline float get_shadow_kernel_radius() const {
        return m_shadow.kernelRadius;
    }
    virtual inline void set_shadow_kernel_radius(float o) {
        m_shadow.kernelRadius = o;
    }

    virtual LightType get_light_type() const {
        return m_lighType;
    }
    virtual inline float get_area() const {
        return m_area;
    }
    virtual inline void set_area(float a) {
        m_area = a;
    }

    virtual Graphics::LightUniforms get_uniforms(Mat4 cameraView) const = 0;
    static uint16_t                 get_non_raytraced_count() {
        return m_nonRaytraceCount;
    }
};

// POINT LIGHT

class PointLight : public Light
{
    float m_effectArea;

    static int m_instanceCount;

  public:
    PointLight(Vec3 color = Vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f)
        : Light("Point Light #" + std::to_string(PointLight::m_instanceCount), LightType::POINT, color, intensity)
        , m_effectArea(12.0f) {
        PointLight::m_instanceCount++;
    }

    inline float get_area_of_effect() const {
        return m_effectArea;
    }
    inline void set_area_of_effect(float a) {
        m_effectArea = a;
    }

    virtual Graphics::LightUniforms get_uniforms(Mat4 cameraView) const;
};

// DIRECTIONAL LIGHT

class DirectionalLight : public Light
{
    Vec3 m_direction;

    static int m_instanceCount;

  public:
    DirectionalLight(Vec3 direction, Vec3 color = Vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f)
        : Light("Directional Light#" + std::to_string(DirectionalLight::m_instanceCount),
                LightType::DIRECTIONAL,
                color,
                intensity)
        , m_direction(direction) {
        DirectionalLight::m_instanceCount++;
    }

    inline Vec3 get_direction() const {
        return m_direction;
    }
    inline void set_direction(Vec3 d) {
        m_direction = d;
    }

    virtual Graphics::LightUniforms get_uniforms(Mat4 cameraView) const;
};
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif