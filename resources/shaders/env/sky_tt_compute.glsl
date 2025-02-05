#shader vertex
#version 460
/*
 * Transmittance LUT
 *
 * In this buffer we precompute the transmittance to the top of the atmosphere.
 * We use the same technique as in "Precomputed Atmospheric Scattering"
 * by Eric Bruneton and Fabrice Neyret (2008).
 */


layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);

    v_uv = uv;
}

#shader fragment
#version 460
#include sky.glsl

layout(location = 0) in  vec2 v_uv;

layout(push_constant) uniform Settings {
    SkySettings sky;
} settings;

layout(location = 0) out vec4 outColor;

void main()
{

    float sun_cos_theta = v_uv.x * 2.0 - 1.0;
    vec3 sun_dir = vec3(-sqrt(1.0 - sun_cos_theta*sun_cos_theta), 0.0, sun_cos_theta);

    float distance_to_earth_center = mix(EARTH_RADIUS, ATMOSPHERE_RADIUS, v_uv.y);
    vec3 ray_origin = vec3(0.0, 0.0, distance_to_earth_center);

    float t_d = ray_sphere_intersection(ray_origin, sun_dir, ATMOSPHERE_RADIUS);
    float dt = t_d / float(TRANSMITTANCE_STEPS);

    vec4 result = vec4(0.0);

    for (int i = 0; i < TRANSMITTANCE_STEPS; ++i) {

        float t = (float(i) + 0.5) * dt;
        vec3 x_t = ray_origin + sun_dir * t;

        float altitude = length(x_t) - EARTH_RADIUS;

        vec4 aerosol_absorption, aerosol_scattering;
        vec4 molecular_absorption, molecular_scattering;
        vec4 extinction;

        get_atmosphere_collision_coefficients(
            altitude,
            settings.sky.month,
            settings.sky.aerosolTurbidity,
            aerosol_absorption, aerosol_scattering,
            molecular_absorption, molecular_scattering,
            extinction);

        result += extinction * dt;
    }


    vec4 transmittance = exp(-result);
    outColor = transmittance;
}

