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
#include <engine/graphics/renderpass.h>
#include <engine/graphics/semaphore.h>
#include <engine/graphics/shaderpass.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/vao.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct CommandPool;

struct CommandBuffer {
    VkCommandBuffer handle      = VK_NULL_HANDLE;
    VkDevice        device      = VK_NULL_HANDLE;
    VkCommandPool   pool        = VK_NULL_HANDLE;
    VkQueue         queue       = VK_NULL_HANDLE;
    bool            isRecording = false;

    void begin( VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );
    void end();
    void reset();
    void submit( Fence fence = {}, std::vector<Semaphore> waitSemaphores = {}, std::vector<Semaphore> signalSemaphores = {} );
    void cleanup();

    /****************************************** */
    /* COMMANDS */
    /****************************************** */

    void begin_renderpass( RenderPass& renderpass, Framebuffer& fbo, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE ) const;
    void end_renderpass( RenderPass& renderpass, Framebuffer& fbo ) const;
    void draw_geometry( VertexArrays& vao, uint32_t instanceCount = 1, uint32_t firstOcurrence = 0, int32_t offset = 0, uint32_t firstInstance = 0 ) const;
    void draw_gui_data() const;
    void bind_shaderpass( ShaderPass& pass ) const;
    void bind_descriptor_set( DescriptorSet         descriptor,
                              uint32_t              ocurrence,
                              ShaderPass&           pass,
                              std::vector<uint32_t> offsets = {},
                              BindingType           binding = BINDING_TYPE_GRAPHIC ) const;
    void set_viewport( Extent2D extent, Offset2D scissorOffset = { 0, 0 } ) const;
    void set_cull_mode( CullingMode mode ) const;
    void set_depth_write_enable( bool op ) const;
    void set_depth_test_enable( bool op ) const;
    void set_depth_bias_enable( bool op ) const;
    void set_depth_bias( float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor ) const;

    void pipeline_barrier( Image&        img,
                           ImageLayout   oldLayout = LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           ImageLayout   newLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           AccessFlags   srcMask   = ACCESS_COLOR_ATTACHMENT_WRITE,
                           AccessFlags   dstMask   = ACCESS_SHADER_READ,
                           PipelineStage srcStage  = STAGE_COLOR_ATTACHMENT_OUTPUT,
                           PipelineStage dstStage  = STAGE_FRAGMENT_SHADER ) const;
    void pipeline_barrier( Image&        img,
                           uint32_t      baseMipLevel,
                           uint32_t      mipLevels,
                           ImageLayout   oldLayout = LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           ImageLayout   newLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           AccessFlags   srcMask   = ACCESS_COLOR_ATTACHMENT_WRITE,
                           AccessFlags   dstMask   = ACCESS_SHADER_READ,
                           PipelineStage srcStage  = STAGE_COLOR_ATTACHMENT_OUTPUT,
                           PipelineStage dstStage  = STAGE_FRAGMENT_SHADER ) const;

    void clear_image( Image& img, ImageLayout layout, ImageAspect aspect = ASPECT_COLOR, Vec4 clearColor = Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ) const;

    /*Copy the entire extent of the image*/
    void blit_image( Image&      srcImage,
                     Image&      dstImage,
                     FilterType  filter    = FILTER_LINEAR,
                     uint32_t    mipLevel  = 0,
                     ImageAspect srcAspect = ASPECT_COLOR,
                     ImageAspect dstAspect = ASPECT_COLOR ) const;

    /*Copy a custom extent of the image*/
    void blit_image( Image&      srcImage,
                     Image&      dstImage,
                     Extent2D    srcOrigin,
                     Extent2D    dstOrigin,
                     Extent2D    srcExtent,
                     Extent2D    dstExtent,
                     FilterType  filter    = FILTER_LINEAR,
                     uint32_t    mipLevel  = 0,
                     ImageAspect srcAspect = ASPECT_COLOR,
                     ImageAspect dstAspect = ASPECT_COLOR ) const;

    void push_constants( ShaderPass& pass, ShaderStageFlags stage, const void* data, uint32_t size, uint32_t offset = 0 ) const;

    void dispatch_compute( Extent3D grid ) const;

    void copy_buffer( Buffer& srcBuffer, Buffer& dstBuffer, size_t size ) const;

    /*
    Expected layout is LAYOUT_UNDEFINED
    */
    void copy_buffer_to_image( Image& img, Buffer& buffer ) const;

    void copy_image_to_buffer( Image& img, Buffer& buffer ) const;

    /*
    Generates mipmaps for a given image following a downsampling by 2 strategy
    */
    void generate_mipmaps( Image& img, ImageLayout initialLayout = LAYOUT_TRANSFER_DST_OPTIMAL, ImageLayout finalLayout = LAYOUT_SHADER_READ_ONLY_OPTIMAL ) const;
};
struct CommandPool {
    VkCommandPool handle = VK_NULL_HANDLE;
    VkDevice      device = VK_NULL_HANDLE;
    VkQueue       queue  = VK_NULL_HANDLE;

    CommandBuffer allocate_command_buffer( uint32_t count, CommandBufferLevel level = COMMAND_BUFFER_LEVEL_PRIMARY );

    void reset( VkCommandPoolResetFlags flags = 0 ) const;

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif