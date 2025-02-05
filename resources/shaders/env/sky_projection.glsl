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

layout(location = 0) in  vec2 v_uv;

layout(set = 0, binding = 1) uniform sampler2D sky;

const float PI                      = 3.14159265358979323846;

void equirectangular_camera(in vec2 fragCoord, out float phi, out float theta)
{
    // vec2 uv = fragCoord / iResolution.xy - 0.5;
    vec2 uv = v_uv - 0.5;
    phi = 2.0 * PI * uv.x;
    theta = PI * uv.y;
}

mat3 rotX(float a) { float s = sin(a), c = cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
mat3 rotY(float a) { float s = sin(a), c = cos(a); return mat3(c,0.,s,0.,1.,0.,-s,0.,c); }
mat3 rotZ(float a) { float s = sin(a), c = cos(a); return mat3(c,-s,0.,s,c,0.,0.,0.,1.); }

// void projection_camera(in vec2 fragCoord, out float phi, out float theta)
// {
//     vec2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;

//     vec3 ray_dir = vec3(1.0 / tan(radians(CAMERA_FOV) * 0.5), uv.x, uv.y);
//     ray_dir = normalize(ray_dir);
//     ray_dir *= rotY(radians(CAMERA_PITCH)) * rotZ(radians(CAMERA_YAW)) * rotX(radians(CAMERA_ROLL));

//     phi = atan(ray_dir.y, ray_dir.x);
//     theta = asin(ray_dir.z);
// }

layout(location = 0) out vec4 outColor;

void main()
{
    float phi, theta;
    equirectangular_camera(gl_FragCoord.xy, phi, theta);
    // projection_camera(fragCoord, phi, theta);

    float azimuth = phi / PI * 0.5 + 0.5;
    // Undo the non-linear transformation from the sky-view LUT
    float elev = sqrt(abs(theta) / (PI * 0.5)) * sign(theta) * 0.5 + 0.5;

    vec3 col = texture(sky, vec2(azimuth, 1.0 -elev)).rgb;

// #if TONEMAPPING_TECHNIQUE == 0
//     // Apply exposure
//     col = col * exp2(EXPOSURE);
//     // Tonemap
//     col = aces_fitted(col);
//     // Apply the sRGB transfer function (gamma correction)
//     col = clamp(gamma_correct(col), 0.0, 1.0);
// #elif TONEMAPPING_TECHNIQUE == 1
//     const float k = 0.05;
//     col = 1.0 - exp(-k * col);
//     col = clamp(gamma_correct(col), 0.0, 1.0);
// #endif

    outColor = vec4(col, 1.0);
}

