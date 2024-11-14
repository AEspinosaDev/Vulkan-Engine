#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

VkCommandPoolCreateInfo Init::command_pool_create_info(uint32_t                 queueFamilyIndex,
                                                       VkCommandPoolCreateFlags flags /*= 0*/) {
    VkCommandPoolCreateInfo info = {};
    info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext                   = nullptr;
    info.flags                   = flags;
    return info;
}

VkCommandBufferAllocateInfo
Init::command_buffer_allocate_info(VkCommandPool        pool,
                                   uint32_t             count /*= 1*/,
                                   VkCommandBufferLevel level /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/) {
    VkCommandBufferAllocateInfo info = {};
    info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext                       = nullptr;
    info.commandPool                 = pool;
    info.commandBufferCount          = count;
    info.level                       = level;
    return info;
}

VkCommandBufferBeginInfo Init::command_buffer_begin_info(VkCommandBufferUsageFlags flags /*= 0*/) {
    VkCommandBufferBeginInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext                    = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags            = flags;
    return info;
}

VkFramebufferCreateInfo Init::framebuffer_create_info(VkRenderPass renderPass, VkExtent2D extent) {
    VkFramebufferCreateInfo info = {};
    info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext                   = nullptr;

    info.renderPass      = renderPass;
    info.attachmentCount = 1;
    info.width           = extent.width;
    info.height          = extent.height;
    info.layers          = 1;

    return info;
}

VkFenceCreateInfo Init::fence_create_info(VkFenceCreateFlags flags /*= 0*/) {
    VkFenceCreateInfo info = {};
    info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext             = nullptr;

    info.flags = flags;

    return info;
}

VkSemaphoreCreateInfo Init::semaphore_create_info(VkSemaphoreCreateFlags flags /*= 0*/) {
    VkSemaphoreCreateInfo info = {};
    info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext                 = nullptr;
    info.flags                 = flags;
    return info;
}

VkSubmitInfo Init::submit_info(VkCommandBuffer* cmd) {
    VkSubmitInfo info = {};
    info.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext        = nullptr;

    info.waitSemaphoreCount   = 0;
    info.pWaitSemaphores      = nullptr;
    info.pWaitDstStageMask    = nullptr;
    info.commandBufferCount   = 1;
    info.pCommandBuffers      = cmd;
    info.signalSemaphoreCount = 0;
    info.pSignalSemaphores    = nullptr;

    return info;
}

VkPresentInfoKHR Init::present_info() {
    VkPresentInfoKHR info = {};
    info.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pNext            = nullptr;

    info.swapchainCount     = 0;
    info.pSwapchains        = nullptr;
    info.pWaitSemaphores    = nullptr;
    info.waitSemaphoreCount = 0;
    info.pImageIndices      = nullptr;

    return info;
}

VkRenderPassBeginInfo
Init::renderpass_begin_info(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer) {
    VkRenderPassBeginInfo info = {};
    info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.pNext                 = nullptr;

    info.renderPass          = renderPass;
    info.renderArea.offset.x = 0;
    info.renderArea.offset.y = 0;
    info.renderArea.extent   = windowExtent;
    info.clearValueCount     = 1;
    info.pClearValues        = nullptr;
    info.framebuffer         = framebuffer;

    return info;
}

VkPipelineShaderStageCreateInfo Init::pipeline_shader_stage_create_info(VkShaderStageFlagBits stage,
                                                                        VkShaderModule        shaderModule) {
    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;

    // shader stage
    info.stage = stage;
    // module containing the code for this shader stage
    info.module = shaderModule;
    // the entry point of the shader
    info.pName = "main";
    return info;
}
VkPipelineVertexInputStateCreateInfo Init::vertex_input_state_create_info() {
    VkPipelineVertexInputStateCreateInfo info = {};
    info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext                                = nullptr;

    // no vertex bindings or attributes
    info.vertexBindingDescriptionCount   = 0;
    info.vertexAttributeDescriptionCount = 0;
    return info;
}

VkPipelineInputAssemblyStateCreateInfo Init::input_assembly_create_info(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext                                  = nullptr;

    info.topology = topology;
    // we are not going to use primitive restart on the entire tutorial so leave it on false
    info.primitiveRestartEnable = VK_FALSE;
    return info;
}
VkPipelineRasterizationStateCreateInfo Init::rasterization_state_create_info(VkPolygonMode   polygonMode,
                                                                             VkCullModeFlags cullMode,
                                                                             VkFrontFace     face,
                                                                             float           lineWidth) {
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext                                  = nullptr;

    info.depthClampEnable = VK_FALSE;
    // rasterizer discard allows objects with holes, default to no
    info.rasterizerDiscardEnable = VK_FALSE;

    info.polygonMode = polygonMode;
    info.lineWidth   = lineWidth;
    // no backface cull
    info.cullMode  = cullMode;
    info.frontFace = face;
    // no depth bias
    info.depthBiasEnable         = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp          = 0.0f;
    info.depthBiasSlopeFactor    = 0.0f;

    return info;
}
VkPipelineMultisampleStateCreateInfo Init::multisampling_state_create_info(VkSampleCountFlagBits samples) {
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext                                = nullptr;

    info.sampleShadingEnable = VK_FALSE;
    // multisampling defaulted to no multisampling (1 sample per pixel)
    info.rasterizationSamples  = samples;
    info.minSampleShading      = 1.0f;
    info.pSampleMask           = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable      = VK_FALSE;
    return info;
}
VkPipelineColorBlendStateCreateInfo Init::color_blend_create_info() {
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext                               = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp       = VK_LOGIC_OP_COPY;

    return colorBlending;
}
VkPipelineColorBlendAttachmentState Init::color_blend_attachment_state(bool enabled) {
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = enabled ? VK_TRUE : VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    return colorBlendAttachment;
}
VkPipelineLayoutCreateInfo Init::pipeline_layout_create_info() {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    // empty defaults
    info.flags                  = 0;
    info.setLayoutCount         = 0;
    info.pSetLayouts            = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges    = nullptr;
    return info;
}

VkPipelineDepthStencilStateCreateInfo
Init::depth_stencil_create_info(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp) {
    VkPipelineDepthStencilStateCreateInfo info = {};
    info.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.pNext                                 = nullptr;

    info.depthTestEnable       = bDepthTest ? VK_TRUE : VK_FALSE;
    info.depthWriteEnable      = bDepthWrite ? VK_TRUE : VK_FALSE;
    info.depthCompareOp        = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
    info.depthBoundsTestEnable = VK_FALSE;
    info.minDepthBounds        = 0.0f; // Optional
    info.maxDepthBounds        = 1.0f; // Optional
    info.stencilTestEnable     = VK_FALSE;

    return info;
}

VkDescriptorSetLayoutBinding Init::descriptorset_layout_binding(VkDescriptorType   type,
                                                                VkShaderStageFlags stageFlags,
                                                                uint32_t           binding,
                                                                uint32_t           descriptorCount) {
    VkDescriptorSetLayoutBinding setbind = {};
    setbind.binding                      = binding;
    setbind.descriptorCount              = descriptorCount;
    setbind.descriptorType               = type;
    setbind.pImmutableSamplers           = nullptr;
    setbind.stageFlags                   = stageFlags;

    return setbind;
}

VkWriteDescriptorSet Init::write_descriptor_buffer(VkDescriptorType        type,
                                                   VkDescriptorSet         dstSet,
                                                   VkDescriptorBufferInfo* bufferInfo,
                                                   uint32_t                binding) {
    VkWriteDescriptorSet write = {};
    write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext                = nullptr;

    write.dstBinding      = binding;
    write.dstSet          = dstSet;
    write.descriptorCount = 1;
    write.descriptorType  = type;
    write.pBufferInfo     = bufferInfo;

    return write;
}

VkImageCreateInfo Init::image_create_info(VkFormat              format,
                                          VkImageUsageFlags     usageFlags,
                                          VkExtent3D            extent,
                                          uint32_t              mipLevels,
                                          VkSampleCountFlagBits samples,
                                          uint32_t              layers,
                                          VkImageCreateFlags    flags) {

    VkImageCreateInfo info = {};
    info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext             = nullptr;
    info.flags             = flags;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels   = mipLevels;
    info.arrayLayers = layers;
    info.samples     = samples;
    info.tiling      = VK_IMAGE_TILING_OPTIMAL;
    info.usage       = usageFlags;

    return info;
}
VkImageViewCreateInfo Init::imageview_create_info(VkFormat           format,
                                                  VkImage            image,
                                                  VkImageViewType    viewType,
                                                  VkImageAspectFlags aspectFlags,
                                                  uint32_t           mipLevels,
                                                  uint32_t           layers) {
    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo info = {};
    info.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext                 = nullptr;

    info.viewType                        = viewType;
    info.image                           = image;
    info.format                          = format;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = mipLevels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = layers;
    info.subresourceRange.aspectMask     = aspectFlags;

    return info;
}

VkSamplerCreateInfo
Init::sampler_create_info(VkFilter             filters,
                          VkSamplerMipmapMode  mipmapMode,
                          float                minLod,
                          float                maxLod,
                          bool                 anysotropicFilter,
                          float                maxAnysotropy,
                          VkSamplerAddressMode samplerAddressMode /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/) {
    VkSamplerCreateInfo info = {};
    info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext               = nullptr;

    info.addressModeU = samplerAddressMode;
    info.addressModeV = samplerAddressMode;
    info.addressModeW = samplerAddressMode;

    info.magFilter = filters;
    info.minFilter = filters;

    info.mipmapMode = mipmapMode;
    info.minLod     = 0.0f; // Optional
    info.maxLod     = maxLod;
    info.mipLodBias = 0.0f; // Optional

    info.anisotropyEnable = anysotropicFilter;
    info.maxAnisotropy    = maxAnysotropy;

    return info;
}
VkWriteDescriptorSet Init::write_descriptor_image(VkDescriptorType       type,
                                                  VkDescriptorSet        dstSet,
                                                  VkDescriptorImageInfo* imageInfo,
                                                  uint32_t               binding) {
    VkWriteDescriptorSet write = {};
    write.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext                = nullptr;

    write.dstBinding      = binding;
    write.dstSet          = dstSet;
    write.descriptorCount = 1;
    write.descriptorType  = type;
    write.pImageInfo      = imageInfo;

    return write;
}

VkAttachmentReference Init::attachment_reference(uint32_t slot, VkImageLayout layout) {
    VkAttachmentReference attachmentRef{};
    attachmentRef.attachment = slot;
    attachmentRef.layout     = layout;

    return attachmentRef;
}

VkViewport Init::viewport(VkExtent2D extent, float minDepth, float maxDepth, float x, float y) {
    VkViewport viewport{};
    viewport.x        = x;
    viewport.y        = y;
    viewport.width    = static_cast<float>(extent.width);
    viewport.height   = static_cast<float>(extent.height);
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    return viewport;
}

VkAccelerationStructureGeometryKHR Init::acceleration_structure_geometry() {
    VkAccelerationStructureGeometryKHR accelerationStructureGeometryKHR{};
    accelerationStructureGeometryKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    return accelerationStructureGeometryKHR;
}

VkAccelerationStructureBuildGeometryInfoKHR Init::acceleration_structure_build_geometry_info() {
    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoKHR{};
    accelerationStructureBuildGeometryInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    return accelerationStructureBuildGeometryInfoKHR;
}

VkAccelerationStructureBuildSizesInfoKHR Init::acceleration_structure_build_sizes_info() {
    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoKHR{};
    accelerationStructureBuildSizesInfoKHR.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    return accelerationStructureBuildSizesInfoKHR;
}

VkWriteDescriptorSetAccelerationStructureKHR Init::write_descriptor_set_acceleration_structure() {
    VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructureKHR{};
    writeDescriptorSetAccelerationStructureKHR.sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    return writeDescriptorSetAccelerationStructureKHR;
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END