/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef INITIALIZERS_H
#define INITIALIZERS_H

#include <engine/common.h>
#include <engine/graphics/utilities/bootstrap.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
namespace Init {

VkCommandPoolCreateInfo     command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
VkCommandBufferBeginInfo    command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);
VkFramebufferCreateInfo     framebuffer_create_info(VkRenderPass renderPass, VkExtent2D extent);
VkFenceCreateInfo           fence_create_info(VkFenceCreateFlags flags = 0);
VkSemaphoreCreateInfo       semaphore_create_info(VkSemaphoreCreateFlags flags = 0);
VkSubmitInfo                submit_info(VkCommandBuffer* cmd);
VkPresentInfoKHR            present_info();
VkRenderPassBeginInfo       renderpass_begin_info(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer);
VkPipelineShaderStageCreateInfo        pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
VkPipelineVertexInputStateCreateInfo   vertex_input_state_create_info();
VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);
VkPipelineRasterizationStateCreateInfo
rasterization_state_create_info(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace face, float lineWidth = 1.0);
VkPipelineMultisampleStateCreateInfo  multisampling_state_create_info(VkSampleCountFlagBits samples);
VkPipelineColorBlendStateCreateInfo   color_blend_create_info();
VkPipelineColorBlendAttachmentState   color_blend_attachment_state(bool enabled);
VkPipelineLayoutCreateInfo            pipeline_layout_create_info();
VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
VkDescriptorSetLayoutBinding descriptorset_layout_binding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding, uint32_t descriptorCount = 1);
VkWriteDescriptorSet         write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
VkImageCreateInfo            image_create_info(VkFormat              format,
                                               VkImageUsageFlags     usageFlags,
                                               VkExtent3D            extent,
                                               uint32_t              mipLevels = 1,
                                               VkSampleCountFlagBits samples   = VK_SAMPLE_COUNT_1_BIT,
                                               uint32_t              layers    = 1,
                                               VkImageType           type      = VK_IMAGE_TYPE_2D,
                                               VkImageCreateFlags    flags     = {});
VkImageViewCreateInfo        imageview_create_info(VkFormat           format,
                                                   VkImage            image,
                                                   VkImageViewType    viewType,
                                                   VkImageAspectFlags aspectFlags,
                                                   uint32_t           mipLevels    = 1,
                                                   uint32_t           layers       = 1,
                                                   uint32_t           baseMipLevel = 0);
VkSamplerCreateInfo          sampler_create_info(VkFilter             filters,
                                                 VkSamplerMipmapMode  mipmapMode,
                                                 float                minLod,
                                                 float                maxLod,
                                                 bool                 anysotropicFilter,
                                                 float                maxAnysotropy,
                                                 VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
VkWriteDescriptorSet         write_descriptor_image(VkDescriptorType       type,
                                                    VkDescriptorSet        dstSet,
                                                    VkDescriptorImageInfo* imageInfos,
                                                    uint32_t               imageCount,
                                                    uint32_t               arraySlot,
                                                    uint32_t               binding);
VkAttachmentReference        attachment_reference(uint32_t slot, VkImageLayout layout);
VkViewport                   viewport(VkExtent2D extent, float minDepth = 0.0f, float maxDepth = 1.0f, float x = 0.0f, float y = 0.0f);

// RAYTRACING RELATED

VkAccelerationStructureGeometryKHR           acceleration_structure_geometry();
VkAccelerationStructureBuildGeometryInfoKHR  acceleration_structure_build_geometry_info();
VkAccelerationStructureBuildSizesInfoKHR     acceleration_structure_build_sizes_info();
VkWriteDescriptorSetAccelerationStructureKHR write_descriptor_set_acceleration_structure();
// VkRayTracingShaderGroupCreateInfoKHR         ray_tracing_shader_group_create_info();
// VkRayTracingPipelineCreateInfoKHR            rayTracingPipelineCreateInfo();

} // namespace Init

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif