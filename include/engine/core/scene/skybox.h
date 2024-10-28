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
namespace Core
{
/*
Skybox for rendering enviroments and IBL
*/
class Skybox
{
  private:
    Geometry *m_box{nullptr};
    TextureHDR *m_env{nullptr};

    // Settings
    float m_blurriness{0.0f};
    float m_intensity{1.0f};
    float m_rotation{0.0f};

    bool m_updateEnviroment{true};

  public:
    Skybox(TextureHDR *env) : m_env(env)
    {
        m_box = Geometry::create_cube();
    }
    ~Skybox()
    {
        delete m_box;
        delete m_env;
    }
    TextureHDR *const get_enviroment_map() const
    {
        return m_env;
    }
    TextureHDR *set_enviroment_map(TextureHDR *env)
    {
        TextureHDR *oldEnv = m_env;
        m_env = env;
        m_updateEnviroment = true;
        return oldEnv;
    }
    Geometry *const get_box() const
    {
        return m_box;
    }
    float get_blurriness() const
    {
        return m_blurriness;
    }
    void set_blurriness(float b)
    {
        m_blurriness = b;
    }
    float get_intensity() const
    {
        return m_intensity;
    }
    void set_intensity(float i)
    {
        m_intensity = i;
    }
    /*
    In degrees
    */
    float get_rotation() const
    {
        return m_rotation;
    }
    /*
    In degrees
    */
    void set_rotation(float r)
    {
        m_rotation = r;
    }
    bool update_enviroment() const
    {
        return m_updateEnviroment;
    }
    void set_update_enviroment(bool i)
    {
        m_updateEnviroment = i;
    }
};

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif