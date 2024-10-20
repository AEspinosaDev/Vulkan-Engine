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

layout(set = 0, binding = 0) uniform sampler2D positionBuffer;
layout(set = 0, binding = 1) uniform sampler2D normalBuffer;
layout(set = 0, binding = 2) uniform sampler2D noise;
layout(set = 0, binding = 3) uniform SampleKernel {
    vec4 samples[64];
} kernel;
layout(set = 0, binding = 4) uniform AuxUniforms {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec2 screenExtent;
    vec2 ssaoParams; //x radius and y bias
} aux;

layout(location = 0) out float outOcclusion;

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
const int KERNEL_SIZE = 64;
float radius = aux.ssaoParams.x;
float bias = aux.ssaoParams.y;

// tile noise texture over screen based on screen dimensions divided by noise size
 vec2 noiseScale = vec2(aux.screenExtent.x/4.0, aux.screenExtent.y/4.0); 


void main()
{
    vec3 position = vec3(texture(positionBuffer, v_uv).rgb);
    vec3 normal =  normalize(texture(normalBuffer,v_uv).rgb * 2.0 - 1.0);
    vec3 randomVec = normalize(texture(noise, v_uv * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * kernel.samples[i].xyz; // from tangent to view-space
        samplePos = position + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = aux.proj * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        // float sampleDepth = texture(depthBuffer, offset.xy).r; // get depth value of kernel sample
          float sampleDepth = texture(positionBuffer, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / KERNEL_SIZE);

   outOcclusion = occlusion;
    // outOcclusion = vec4(0.0f,1.0f,0.0f,1.0f);
}

