#ifndef VK_LIGHT
#define VK_LIGHT

#include "../private/vk_uniforms.h"
#include "../vk_object3D.h"

namespace vke
{

    class Light : public Object3D
    {
    protected:
        glm::vec3 m_color;
        float m_intensity;

        bool m_castShadows{true};
        // Image m_shadowMap | Texture m_shadowMap;
        // DescriptorSet m_textureDescriptor;

        // static int m_shadowResolution{1080};

        friend class Renderer;

        virtual LightUniforms get_uniforms() const = 0;

    public:
        Light(glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f) : Object3D(LIGHT), m_color(color), m_intensity(intensity) {}

        virtual inline glm::vec3 get_color() const { return m_color; }
        virtual inline void set_color(glm::vec3 c) { m_color = c; }

        virtual inline float get_intensity() const { return m_intensity; }
        virtual inline void set_intensity(float i) { m_intensity = i; }

        virtual inline bool get_cast_shadows() const { return m_castShadows; }
        virtual inline void set_cast_shadows(bool o) { m_castShadows = o; }
    };

    // POINT LIGHT

    class PointLight : public Light
    {
        float m_effectArea;
        float m_decaying;

        virtual LightUniforms get_uniforms() const;

    public:
        PointLight(glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f) : Light(color, intensity), m_effectArea(10.0f), m_decaying(1.0f) {}

        inline float get_area_of_effect() const { return m_effectArea; }
        inline void set_area_of_effect(float a) { m_effectArea = a; }

        inline float get_decaying() const { return m_decaying; }
        inline void set_decaying(float d) { m_decaying = d; }
    };

    // DIRECTIONAL LIGHT

    class DirectionalLight : public Light
    {
        glm::vec3 m_direction;

        virtual LightUniforms get_uniforms() const;

    public:
        DirectionalLight(glm::vec3 direction, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f) : Light(color, intensity), m_direction(direction) {}

        inline glm::vec3 get_direction() const { return m_direction; }
        inline void set_direction(glm::vec3 d) { m_direction = d; }
    };

}

#endif