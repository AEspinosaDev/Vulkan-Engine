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

/*
Settings for Procedural Sky Rendering
*/
struct SkySettings {
    float          sunElevationDeg  = 45.0f; // 0=horizon, 90=zenith
    uint32_t       month            = 0;     // 0-11, January to December
    float          altitude         = 0.5f;  // km
    SkyAerosolType aerosol          = SkyAerosolType::URBAN;
    Vec4           groundAlbedo     = Vec4(0.3);
    float          aerosolTurbidity = 1.0f;
    uint32_t       resolution       = 512;
    bool           useForIBL        = true;
    UpdateType     updateType       = UpdateType::ON_DEMAND;
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
    SkySettings    m_proceduralSky        = {};

    // Query
    bool m_updateEnviroment      = true; // For updating enviroment texture and cubemaps
    bool m_updateTransmitanceLUT = true; // For updating proc.sky transmittance LUT
    bool m_updateSky             = true; // For updating proc.sky texture
    bool m_active                = true;

  public:
    Skybox(TextureHDR* env)
        : m_env(env) {
        m_box = Geometry::create_simple_cube();
    }
    Skybox()
        : m_env(nullptr) {
        m_box = Geometry::create_simple_cube();
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
    inline void set_sky_type(EnviromentType type) {
        m_updateEnviroment = true;
        m_envType          = type;
    }
    inline EnviromentType get_sky_type() const {
        return m_envType;
    }
    /*
    Procedural Sky
    */
    inline void set_sky_settings(SkySettings settings) {
        m_proceduralSky    = settings;
        m_updateEnviroment = true;
    }
    /*
    Procedural Sky
    */
    inline SkySettings get_sky_settings() const {
        return m_proceduralSky;
    }

    inline bool update_enviroment() const {
        return m_updateEnviroment;
    }
    inline void update_enviroment(bool i) {
        m_updateEnviroment = i;
    }
    inline bool update_sky_transmitance() const {
        return m_updateTransmitanceLUT;
    }
    inline void update_sky_transmitance(bool i) {
        m_updateTransmitanceLUT = i;
    }
    inline bool update_sky() const {
        return m_updateSky;
    }
    inline void update_sky(bool i) {
        m_updateSky = i;
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif