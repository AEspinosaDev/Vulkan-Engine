

#ifndef PI 
#define PI              3.1415926535897932384626433832795
#endif
#ifndef EPSILON 
#define EPSILON         0.001
#endif  

struct                  SubsurfaceBSDF{
    vec3                albedo;
    vec3                normal;
    float               roughness;
    float               ao;
    float               depth;
};


float random (vec2 st) 
{
  return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

vec3 Rr(vec3 d, float radius) // Diffusion profile computation
{
  return (exp(-radius / d) + exp(-radius / (d * 3.0))) / (8.0 * PI * d);
}

float Pr(float d, float radius) // Diffusion profile PDF computation
{
  return (exp(-radius / d) + exp(-radius / (d * 3.0))) / (8.0 * PI * d);
}

vec3 computeSSSS(vec3 wi,
    vec3 wo, 
    vec3 radiance, 
    vec2 uv,
    Sampler2D textureDepth,
    Sampler2D textureMaterial,
    Sampler2D textureScattering,
    SubsurfaceBSDF bsdf)
{ 
  vec3 sssDiffuse = vec3(0.0);
  vec3 diffusion = vec3(0.0);
  vec3 weight = vec3(0.0);

  // Load from textures
  vec3 A = texture(textureAlbedo, uv).rgb;
  vec3 diffuse = A * texture(textureDiffuseLight, uv).rgb;
  vec3 scatteringDistance = texture(textureScatteringDistance, uv).rgb;
  float mask = texture(textureMask, uv).r;
  float thickness = texture(textureThickness, uv).g;
  
  // Render world scale & distance LOD for scattering distance
  float camDist = length(texture(texturePosition, uv).xyz);
  float distAttFactor = 1.0 / pow(camDist + EPSILON, 2.0);
  vec3 d = clamp(mix(scatteringDistance * worldUnitScale * camDist * distAttFactor, scatteringDistance * worldUnitScale, testScene), EPSILON, 1.0);
  float maxRadius = max(d.r, max(d.g, d.b));

  // World fragment depth computation
  float depth = texture(textureDepth, uv).r;
  vec2 fragCoords = uv * 2.0 - vec2(1.0);
  vec3 fragViewPosition = (invProjection * (depth * vec4(fragCoords, 0.0, 1.0))).xyz;
  vec3 fragWorldPosition = (invView * vec4(fragViewPosition, 1.0)).xyz;

  // To avoid periodic pattern sampling
  float jitter = 2.0 * PI * random(mod(uv * screenSize, float(sampleCount))); 

  for (int i = 0; i < sampleCount; i++)
  {
    // Define polar coordinates to sample on a disk around the corresponding UVs
    float theta = samples[i].x + jitter;
    float r = samples[i].y * maxRadius;
    vec2 sampleCoords = vec2(cos(theta) * r, sin(theta) * r);
    sampleCoords = ((projection * vec4(fragViewPosition + vec3(sampleCoords, 0.0), 1.0)) / depth).xy;
    vec2 sampleUV = (sampleCoords + 1.0) * 0.5;

    // World radial distance computation
    float sampleDepth = texture(textureDepth, sampleUV).r;
    vec3 sampleViewPosition = (invProjection * (sampleDepth * vec4(sampleCoords, 0.0, 1.0))).xyz;
    vec3 sampleWorldPosition = (invView * vec4(sampleViewPosition, 1.0)).xyz;
    float radialDistance = max(distance(sampleWorldPosition, fragWorldPosition), EPSILON);

    // Diffusion computation
    vec3 rRr = Rr(d, radialDistance); // We use r*R(r) instead of simply R(r) to integrate the disk
    float pr = Pr(maxRadius, r);
    diffusion = rRr / pr;

    // Multiple Scattering contribution
    sssDiffuse += diffusion * texture(textureDiffuseLight, sampleUV).rgb;
    weight += diffusion;
    
    // Single Scattering contribution
    vec3 transmittance = scatteringDistance * exp(-attenuationCoeff * thickness); // Translucency color (which is the scattering distance base color without scaling) * Beer-Lambert law attenuation
    sssDiffuse += mix(transmittance * diffusion * texture(textureDiffuseBackLight, sampleUV).rgb, vec3(0.0), testScene);
  }

  // Divide by sum of weights to renormalize
  sssDiffuse = sssDiffuse / max(weight, EPSILON);

  // Blush (Hacky method)
  float maxCoeff = max(max(diffuse.r, diffuse.g), diffuse.b);
  vec3 maxChannel = step(maxCoeff, diffuse);
  vec3 blush = mix(maxCoeff * maxChannel * (1.0 - thickness) * blushScale, vec3(0.0), testScene);

  return mix(diffuse, (A + blush) * sssDiffuse, mask);
}