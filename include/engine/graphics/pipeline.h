/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PIPELINE_H
#define PIPELINE_H

#include <engine/common.h>
#include <engine/graphics/descriptors.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

// GENERAL PIPELINE SETTINGS
struct PipelineSettings {

    std::unordered_map<int, bool> descriptorSetLayoutIDs;
    std::vector<PushConstant>     pushConstants = {};
};
// GRAPHIC PIPELINE SETTINGS
struct GraphicPipelineSettings {
    std::unordered_map<int, bool>                    attributes;
    VkPrimitiveTopology                              topology         = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode                                    poligonMode      = VK_POLYGON_MODE_FILL;
    VkCullModeFlagBits                               cullMode         = VK_CULL_MODE_NONE;
    VkFrontFace                                      drawOrder        = VK_FRONT_FACE_CLOCKWISE;
    VkSampleCountFlagBits                            samples          = VK_SAMPLE_COUNT_1_BIT;
    bool                                             sampleShading    = true;
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments = {Init::color_blend_attachment_state(false)};
    bool                                             depthTest        = true;
    bool                                             depthWrite       = true;
    VkCompareOp                                      depthOp          = VK_COMPARE_OP_LESS_OR_EQUAL;
    std::vector<VkDynamicState>                      dynamicStates    = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
};
/*
/Pipeline data and creation wrapper
*/
namespace PipelineBuilder {
void build_pipeline_layout(VkPipelineLayout& layout,
                           VkDevice          device,
                           DescriptorPool    descriptorManager,
                           PipelineSettings& settings);

void build_graphic_pipeline(VkPipeline&                                  pipeline,
                            VkPipelineLayout&                            layout,
                            VkDevice                                     device,
                            VkRenderPass                                 renderPass,
                            VkExtent2D                                   extent,
                            GraphicPipelineSettings&                     settings,
                            std::vector<VkPipelineShaderStageCreateInfo> shaderStages);

void build_compute_pipeline(VkPipeline&                     pipeline,
                            VkPipelineLayout&               layout,
                            VkDevice                        device,
                            VkPipelineShaderStageCreateInfo computeStage);
}; // namespace PipelineBuilder

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif