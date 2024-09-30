/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H
#include <engine/graphics/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class ShadowPass : public RenderPass
{
    DepthFormatType m_depthFormat;

public:
    ShadowPass(VkExtent2D extent,
               uint32_t framebufferCount,
               uint32_t numLights,
               DepthFormatType depthFormat) : RenderPass(extent, framebufferCount, numLights),
                                              m_depthFormat(depthFormat) {}

    void init(VkDevice &device);
    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void init_resources(VkDevice &device,
                        VkPhysicalDevice &gpu,
                        VmaAllocator &memory,
                        VkQueue &gfxQueue,
                        utils::UploadContext &uploadContext);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void update(VkDevice& device, VmaAllocator& memory, Swapchain* swp = nullptr);
};

VULKAN_ENGINE_NAMESPACE_END

#endif
