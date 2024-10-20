// /*
//     This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

//     MIT License

//     Copyright (c) 2023 Antonio Espinosa Garcia

// */
// #ifndef GEOMETRY_PASS_H
// #define GEOMETRY_PASS_H
// #include <engine/graphics/renderpass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN

// class GeometryPass : public RenderPass
// {
//     DepthFormatType m_depthFormat;

// public:
//     GeometryPass(Context *ctx, VkExtent2D extent,
//                  uint32_t framebufferCount,
//                  DepthFormatType depthFormat) : RenderPass(ctx, extent, framebufferCount, 1),
//                                                 m_depthFormat(depthFormat) {}
//     void init();

//     void create_pipelines(DescriptorManager &descriptorManager);

//     void init_resources();

//     void render( uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

//     void update();

//     void create_g_buffer_samplers();

//     void set_g_buffer_clear_color(Vec4 color);
// };
// VULKAN_ENGINE_NAMESPACE_END

// #endif