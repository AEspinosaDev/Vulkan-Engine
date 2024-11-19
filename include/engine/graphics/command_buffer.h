/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include <engine/common.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/framebuffer.h>
#include <engine/graphics/semaphore.h>
#include <engine/graphics/shaderpass.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/vao.h>
#include <engine/graphics/vk_renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct CommandPool;

struct CommandBuffer {
    VkCommandBuffer handle      = VK_NULL_HANDLE;
    VkDevice        device      = VK_NULL_HANDLE;
    VkCommandPool   pool        = VK_NULL_HANDLE;
    VkQueue         queue       = VK_NULL_HANDLE;
    bool            isRecording = false;

    void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    void end();
    void reset();
    void submit(Fence fence, std::vector<Semaphore> waitSemaphores = {}, std::vector<Semaphore> signalSemaphores = {});
    void cleanup();

    /****************************************** */
    /* COMMANDS */
    /****************************************** */

    void begin_renderpass(VulkanRenderPass& renderpass,
                          Framebuffer&      fbo,
                          VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
    void end_renderpass();
    void draw_geometry(VertexArrays& vao,
                       uint32_t      instanceCount  = 1,
                       uint32_t      firstOcurrence = 0,
                       int32_t       offset         = 0,
                       uint32_t      firstInstance  = 0);
    void draw_gui_data();
    void bind_shaderpass(ShaderPass& pass, BindingType binding = BINDING_TYPE_GRAPHIC);
    void bind_descriptor_set(DescriptorSet         descriptor,
                             uint32_t              ocurrence,
                             ShaderPass&           pass,
                             std::vector<uint32_t> offsets = {},
                             BindingType           binding = BINDING_TYPE_GRAPHIC);
    void set_viewport(Extent2D extent, Offset2D scissorOffset = {0, 0});
    void set_cull_mode(CullingMode mode);
    void set_depth_write_enable(bool op);
    void set_depth_test_enable(bool op);
    void set_depth_bias_enable(bool op);
    void set_depth_bias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
};
struct CommandPool {
    VkCommandPool handle = VK_NULL_HANDLE;
    VkDevice      device = VK_NULL_HANDLE;
    VkQueue       queue  = VK_NULL_HANDLE;

    CommandBuffer allocate_command_buffer(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void reset(VkCommandPoolResetFlags flags = 0) const;

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif