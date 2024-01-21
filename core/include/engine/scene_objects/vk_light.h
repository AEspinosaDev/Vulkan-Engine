#ifndef VK_LIGHT
#define VK_LIGHT

#include "../private/vk_uniforms.h"
#include "../private/vk_descriptors.h"
#include "../vk_texture.h"
#include "../vk_object3D.h"

namespace vke
{

    class Light : public Object3D
    {
    protected:
        glm::vec3 m_color;
        float m_intensity;

        // Shadows
        struct Shadow
        {
            bool cast{true};
            float nearPlane{0.5f};
            float farPlane{100.0f};
            float fov{45.0f};
            glm::vec3 target{0.0f, 0.0f, 0.0f};

            Texture *map;
            DescriptorSet descriptor;
        };

        Shadow m_shadow;
        const LightType m_lighType;

        friend class Renderer;

        virtual LightUniforms get_uniforms() const = 0;

    public:
        Light(std::string name, LightType type, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f) : Object3D(name, LIGHT), m_color(color),
                                                                                                                         m_intensity(intensity), m_lighType(type) {}

        virtual inline glm::vec3 get_color() const { return m_color; }
        virtual inline void set_color(glm::vec3 c) { m_color = c; }

        virtual inline float get_intensity() const { return m_intensity; }
        virtual inline void set_intensity(float i) { m_intensity = i; }

        virtual inline bool get_cast_shadows() const { return m_shadow.cast; }
        virtual inline void set_cast_shadows(bool o) { m_shadow.cast = o; }

        virtual inline glm::vec3 get_shadow_target() const { return m_shadow.target; }
        virtual inline void set_shadow_target(glm::vec3 o) { m_shadow.target = o; }

        virtual inline float get_shadow_near() const { return m_shadow.nearPlane; }
        virtual inline void set_shadow_near(float n) { m_shadow.nearPlane = n; }

        virtual inline float get_shadow_far() const { return m_shadow.farPlane; }
        virtual inline void set_shadow_far(float f) { m_shadow.farPlane = f; }

        virtual inline float get_shadow_fov() const { return m_shadow.fov; }
        virtual inline void set_shadow_fov(float f) { m_shadow.fov = f; }

        // Read only
        virtual const Texture *const get_shadow_map() const { return m_shadow.map; }
        virtual LightType get_light_type() const { return m_lighType; }
    };

    // POINT LIGHT

    class PointLight : public Light
    {
        float m_effectArea;
        float m_decaying;

        virtual LightUniforms get_uniforms() const;

    public:
        PointLight(glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 0.1f) : Light("Point Light", LightType::POINT, color, intensity), m_effectArea(12.0f), m_decaying(1.0f) {}

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
        DirectionalLight(glm::vec3 direction, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f), float intensity = 1.0f) : Light("Directional Light", LightType::DIRECTIONAL, color, intensity), m_direction(direction) {}

        inline glm::vec3 get_direction() const { return m_direction; }
        inline void set_direction(glm::vec3 d) { m_direction = d; }
    };

}

#endif