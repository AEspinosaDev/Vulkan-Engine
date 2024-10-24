/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SKYBOX_H
#define SKYBOX_H

#include <engine/core/geometries/geometry.h>
#include <engine/core/scene/object3D.h>
#include <engine/core/textures/texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Core
{
/*
Skybox for rendering enviroments and IBL
*/
class Skybox : public Object3D
{
  private:
    Geometry *m_box{nullptr};
    TextureHDR *m_env{nullptr};

    Graphics::Image m_envCubemap;
    Graphics::Image m_irrCubemap;

    float m_blurriness{0.0f};
    float m_intensity{1.0f};
    bool m_updateEnviroment{true};

    static int m_instanceCount;

    friend Graphics::Image *const get_env_cubemap(Skybox *skb);
    friend Graphics::Image *const get_irr_cubemap(Skybox *skb);

  public:
    Skybox(TextureHDR *env) : Object3D("Skybox #" + std::to_string(Skybox::m_instanceCount), SKYBOX), m_env(env)
    {
        Skybox::m_instanceCount++;
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
    bool update_enviroment() const
    {
        return m_updateEnviroment;
    }
    void set_update_enviroment(bool i)
    {
       m_updateEnviroment = i;
    }
};

Graphics::Image *const get_env_cubemap(Skybox *skb);
Graphics::Image *const get_irr_cubemap(Skybox *skb);

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END

#endif