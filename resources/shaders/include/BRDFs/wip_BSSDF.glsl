

#ifndef PI 
#define PI              3.1415926535897932384626433832795
#endif
#ifndef EPSILON 
#define EPSILON         0.001
#endif  

struct DisneyBSSDF {
  vec3 albedo;
  float opacity;
  vec3 normal;
  float mask;
  float thickness;
  vec3 scatteringDistance;
    // float               roughness;
    // float               ao;
};

struct DisneyBSSDFSettings {
  int sampleCount;
  float worldUnitScale;
  float frameCounter;
  float testScene;
  vec2 samples[32];
};

float random(vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 Rr(vec3 d, float radius) // Diffusion profile computation
{
  return (exp(-radius / d) + exp(-radius / (d * 3.0))) / (8.0 * PI * d);
}

float Pr(float d, float radius) // Diffusion profile PDF computation
{
  return (exp(-radius / d) + exp(-radius / (d * 3.0))) / (8.0 * PI * d);
}

vec3 evalDisneyBSSDF(
  vec3 viewPos,
  vec3 modelPos,
  vec3 radiance,
  vec2 uv,
  float camDist,
  sampler2D texturePosition,
  sampler2D textureDiffuseLight,
  sampler2D textureDiffuseBackLight,
  DisneyBSSDF bsdf,
  DisneyBSSDFSettings settings
) {

  const float EPS = 0.0001;

    //SETTINGS
  const int sampleCount = 4;
  const float worldUnitScale = 1.0;
  const float frameCounter = 5.0;
  const float testScene = 5.0;
  const float attenuationCoeff = 2.5; // The parameter to fit for single scatter
  const float blushScale = 0.4;

  vec3 sssDiffuse = vec3(0.0);
  vec3 diffusion = vec3(0.0);
  vec3 weight = vec3(0.0);

  // Diffuse Term
  vec3 diffuse = bsdf.albedo * texture(textureDiffuseLight, uv).rgb;

  // Render world scale & distance LOD for scattering distance
  float distAttFactor = 1.0 / pow(camDist + EPS, 2.0);
  vec3 d = clamp(mix(bsdf.scatteringDistance * worldUnitScale * camDist * distAttFactor, bsdf.scatteringDistance * worldUnitScale, testScene), EPS, 1.0);
  float maxRadius = max(d.r, max(d.g, d.b));

  // World fragment depth computation
  float depth = texture(texturePosition, uv).a;
  // vec2 fragCoords = uv * 2.0 - vec2(1.0);
  // vec3 fragViewPosition = (invProjection * (depth * vec4(fragCoords, 0.0, 1.0))).xyz;
  // vec3 fragWorldPosition = (invView * vec4(fragViewPosition, 1.0)).xyz;
  vec3 fragViewPosition = viewPos;
  vec3 fragWorldPosition = modelPos;

  // To avoid periodic pattern sampling
  float jitter = 2.0 * PI * random(mod(uv * vec2(textureSize(texturePosition, 0)), float(sampleCount)));

  for(int i = 0; i < sampleCount; i++) {
    // Define polar coordinates to sample on a disk around the corresponding UVs
    float theta = settings.samples[i].x + jitter;
    float r = settings.samples[i].y * maxRadius;
    vec2 sampleCoords = vec2(cos(theta) * r, sin(theta) * r);
    // sampleCoords = ((projection * vec4(fragViewPosition + vec3(sampleCoords, 0.0), 1.0)) / depth).xy;
    vec2 sampleUV = (sampleCoords + 1.0) * 0.5;

    // World radial distance computation
    // float sampleDepth = texture(textureDepth, sampleUV).r;
    // vec3 sampleViewPosition = (invProjection * (sampleDepth * vec4(sampleCoords, 0.0, 1.0))).xyz;
    // vec3 sampleWorldPosition = (invView * vec4(sampleViewPosition, 1.0)).xyz;
    vec3 sampleViewPosition = vec3(0.0);
    vec3 sampleWorldPosition = vec3(0.0);
    float radialDistance = max(distance(sampleWorldPosition, fragWorldPosition), EPS);

    // Diffusion computation
    vec3 rRr = Rr(d, radialDistance); // We use r*R(r) instead of simply R(r) to integrate the disk
    float pr = Pr(maxRadius, r);
    diffusion = rRr / pr;

    // Multiple Scattering contribution
    sssDiffuse += diffusion * texture(textureDiffuseLight, sampleUV).rgb;
    weight += diffusion;

    // Single Scattering contribution
    vec3 transmittance = bsdf.scatteringDistance * exp(-attenuationCoeff * bsdf.thickness); // Translucency color (which is the scattering distance base color without scaling) * Beer-Lambert law attenuation
    sssDiffuse += mix(transmittance * diffusion * texture(textureDiffuseBackLight, sampleUV).rgb, vec3(0.0), testScene);
  }

  // Divide by sum of weights to renormalize
  sssDiffuse = sssDiffuse / max(weight, EPS);

  // Blush (Hacky method)
  float maxCoeff = max(max(diffuse.r, diffuse.g), diffuse.b);
  vec3 maxChannel = step(maxCoeff, diffuse);
  vec3 blush = mix(maxCoeff * maxChannel * (1.0 - bsdf.thickness) * blushScale, vec3(0.0), testScene);

  return mix(diffuse, (bsdf.albedo + blush) * sssDiffuse, bsdf.mask);
}