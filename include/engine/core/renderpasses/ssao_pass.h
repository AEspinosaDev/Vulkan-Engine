// /*
//     This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

//     MIT License

//     Copyright (c) 2023 Antonio Espinosa Garcia

// */
// #ifndef SSAO_PASS_H
// #define SSAO_PASS_H
// #include <random>
// #include <engine/graphics/renderpass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN

// class SSAOPass : public RenderPass
// {
//     Mesh *m_vignette;

//     DescriptorManager m_descriptorManager{};

//     DescriptorSet m_descriptorSet{};
//     Buffer m_kernelBuffer{};
//     Buffer m_auxBuffer{};

//     Texture *m_noiseTexture{nullptr};

//     Image m_positionBuffer;
//     Image m_normalsBuffer;

// public:
//     SSAOPass(Context *ctx, VkExtent2D extent,
//              uint32_t framebufferCount,
//              Mesh *vignette) : RenderPass(ctx, extent, framebufferCount),
//                                m_vignette(vignette) {}

//     void init();

//     void create_descriptors();

//     void create_pipelines(DescriptorManager &descriptorManager);

//     void init_resources();

//     void render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

//     void cleanup();

//     void set_g_buffer(Image position, Image normals);

//     void update_uniforms(CameraUniforms &cameraUniforms, Vec2 ssaoParams, size_t size);

//     void update();
// };

// VULKAN_ENGINE_NAMESPACE_END

// #endif
