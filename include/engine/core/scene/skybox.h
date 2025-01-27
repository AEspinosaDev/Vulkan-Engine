/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SKYBOX_H
#define SKYBOX_H

#include <engine/core/geometries/geometry.h>
#include <engine/core/scene/object3D.h>
#include <engine/core/textures/textureHDR.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core {

enum EnviromentType
{
    IMAGE_BASED_ENV = 0,
    PROCEDURAL_ENV  = 1,
};
/*
Skybox for rendering enviroments and IBL
*/
class Skybox
{
  private:
    Geometry*   m_box = nullptr;
    TextureHDR* m_env = nullptr;

    // Settings
    float          m_blurriness           = 0.0f;
    float          m_intensity            = 1.0f;
    float          m_rotation             = 0.0f;
    uint32_t       m_irradianceResolution = 32;
    EnviromentType m_envType              = IMAGE_BASED_ENV;
    float          m_time;
    uint32_t       m_month;
    uint32_t       m_aerosolType;

    bool m_updateEnviroment{true};
    bool m_active{true};

  public:
    Skybox(TextureHDR* env)
        : m_env(env) {
        m_box = Geometry::create_cube();
    }
    Skybox()
        : m_env(nullptr) {
        m_box = Geometry::create_cube();
    }
    ~Skybox() {
        delete m_box;
        delete m_env;
    }
    TextureHDR* const get_enviroment_map() const {
        return m_env;
    }
    TextureHDR* set_enviroment_map(TextureHDR* env) {
        TextureHDR* oldEnv = m_env;
        m_env              = env;
        m_updateEnviroment = true;
        return oldEnv;
    }
    Geometry* const get_box() const {
        return m_box;
    }
    inline float get_blurriness() const {
        return m_blurriness;
    }
    inline void set_blurriness(float b) {
        m_blurriness = b;
    }
    inline float get_intensity() const {
        return m_intensity;
    }
    inline void set_color_intensity(float i) {
        m_intensity = i;
    }
    /*
    In degrees
    */
    inline float get_rotation() const {
        return m_rotation;
    }
    /*
    In degrees
    */
    inline void set_rotation(float r) {
        m_rotation = r;
    }
    inline bool update_enviroment() const {
        return m_updateEnviroment;
    }
    inline void set_update_enviroment(bool i) {
        m_updateEnviroment = i;
    }
    inline void set_active(const bool s) {
        m_active = s;
    }
    inline bool is_active() {
        return m_active;
    }
    inline uint32_t get_irradiance_resolution() const {
        return m_irradianceResolution;
    }
    inline void set_irradiance_resolution(uint32_t r) {
        m_irradianceResolution = r;
        m_updateEnviroment     = true;
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif