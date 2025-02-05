#shader vertex
#version 460

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
layout(set = 0, binding = 0) uniform sampler2D transmitanceLUT;

layout(location = 0) out vec4 outColor;

vec4 compute_inscattering(vec3 ray_origin, vec3 ray_dir, float t_d, out vec4 transmittance)
{
    vec3 sun_dir = get_sun_direction(settings.sky.sunElevationDeg);
    float cos_theta = dot(-ray_dir, sun_dir);

    float molecular_phase = molecular_phase_function(cos_theta);
    float aerosol_phase = aerosol_phase_function(cos_theta);

    float dt = t_d / float(IN_SCATTERING_STEPS);

    vec4 L_inscattering = vec4(0.0);
    transmittance = vec4(1.0);

    for (int i = 0; i < IN_SCATTERING_STEPS; ++i) {
        float t = (float(i) + 0.5) * dt;
        vec3 x_t = ray_origin + ray_dir * t;

        float distance_to_earth_center = length(x_t);
        vec3 zenith_dir = x_t / distance_to_earth_center;
        float altitude = distance_to_earth_center - EARTH_RADIUS;
        float normalized_altitude = altitude / ATMOSPHERE_THICKNESS;

        float sample_cos_theta = dot(zenith_dir, sun_dir);

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

        vec4 transmittance_to_sun = transmittance_from_lut(
            transmitanceLUT, sample_cos_theta, normalized_altitude);

        vec4 ms = get_multiple_scattering(
            transmitanceLUT, settings.sky.groundAlbedo, sample_cos_theta, normalized_altitude,
            distance_to_earth_center);

        vec4 S = sun_spectral_irradiance *
            (molecular_scattering * (molecular_phase * transmittance_to_sun + ms) +
             aerosol_scattering   * (aerosol_phase   * transmittance_to_sun + ms));

        vec4 step_transmittance = exp(-dt * extinction);

        // Energy-conserving analytical integration
        // "Physically Based Sky, Atmosphere and Cloud Rendering in Frostbite"
        // by Sébastien Hillaire
        vec4 S_int = (S - S * step_transmittance) / max(extinction, 1e-7);
        L_inscattering += transmittance * S_int;
        transmittance *= step_transmittance;
    }

    return L_inscattering;
}

void main()
{
    
    float azimuth = 2.0 * PI * v_uv.x;

    // Apply a non-linear transformation to the elevation to dedicate more
    // texels to the horizon, where having more detail matters.
    float l = v_uv.y * 2.0 - 1.0;
    float elev = l*l * sign(l) * PI * 0.5; // [-pi/2, pi/2]

    vec3 ray_dir = vec3(cos(elev) * cos(azimuth),
                        cos(elev) * sin(azimuth),
                        sin(elev));

    const float EYE_DISTANCE_TO_EARTH_CENTER    = EARTH_RADIUS + settings.sky.altitude;
    vec3 ray_origin = vec3(0.0, 0.0, EYE_DISTANCE_TO_EARTH_CENTER);

    float atmos_dist  = ray_sphere_intersection(ray_origin, ray_dir, ATMOSPHERE_RADIUS);
    float ground_dist = ray_sphere_intersection(ray_origin, ray_dir, EARTH_RADIUS);
    float t_d;
    if (settings.sky.altitude < ATMOSPHERE_THICKNESS) {
        // We are inside the atmosphere
        if (ground_dist < 0.0) {
            // No ground collision, use the distance to the outer atmosphere
            t_d = atmos_dist;
        } else {
            // We have a collision with the ground, use the distance to it
            t_d = ground_dist;
        }
    } else {
        // We are in outer space
        if (atmos_dist < 0.0) {
            // No collision with the atmosphere, just return black
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        } else {
            // Move the ray origin to the atmosphere intersection
            ray_origin = ray_origin + ray_dir * (atmos_dist + 1e-3);
            if (ground_dist < 0.0) {
                // No collision with the ground, so the ray is exiting through
                // the atmosphere.
                float second_atmos_dist = ray_sphere_intersection(
                    ray_origin, ray_dir, ATMOSPHERE_RADIUS);
                t_d = second_atmos_dist;
            } else {
                t_d = ground_dist - atmos_dist;
            }
        }
    }

    vec4 transmittance;
    vec4 L = compute_inscattering(ray_origin, ray_dir, t_d, transmittance);

#if ENABLE_SPECTRAL == 1
    outColor = vec4(linear_srgb_from_spectral_samples(L), 1.0);
#else
    outColor = vec4(L.rgb, 1.0);
#endif
}

