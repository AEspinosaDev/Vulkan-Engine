#include <engine/graphics/pipeline.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void PipelineBuilder::build_pipeline_layout(VkPipelineLayout& layout,
                                            VkDevice&         device,
                                            DescriptorPool&   descriptorManager,
                                            PipelineSettings& settings) {
    std::vector<VkDescriptorSetLayout> descriptorLayouts;
    for (auto& layoutID : settings.descriptorSetLayoutIDs)
    {
        if (layoutID.second)
            descriptorLayouts.push_back(descriptorManager.get_layout(layoutID.first));
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = Init::pipeline_layout_create_info();
    pipelineLayoutInfo.setLayoutCount             = (uint32_t)descriptorLayouts.size();
    pipelineLayoutInfo.pSetLayouts                = descriptorLayouts.data();

    if (!settings.pushConstants.empty())
    {
        pipelineLayoutInfo.pushConstantRangeCount = settings.pushConstants.size();
        pipelineLayoutInfo.pPushConstantRanges    = settings.pushConstants.data();
    }

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
    {
        throw new VKException("failed to create pipeline "
                              "layout!");
    }
}
void PipelineBuilder::build_graphic_pipeline(VkPipeline&                                  pipeline,
                                             VkPipelineLayout&                            layout,
                                             VkDevice&                                    device,
                                             VkRenderPass&                                renderPass,
                                             VkExtent2D&                                  extent,
                                             PipelineSettings&                            settings,
                                             std::vector<VkPipelineShaderStageCreateInfo> shaderStages) {

    // Vertex and geometry
    VkPipelineVertexInputStateCreateInfo   vertexInputInfo = Init::vertex_input_state_create_info();
    VkPipelineInputAssemblyStateCreateInfo inputAssembly   = Init::input_assembly_create_info(settings.topology);

    auto bindingDescription                       = Utils::Vertex::getBindingDescription();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions    = &bindingDescription;

    auto attributeDescriptions =
        Utils::Vertex::getAttributeDescriptions(settings.attributes[VertexAttributeType::POSITION],
                                                settings.attributes[VertexAttributeType::NORMAL],
                                                settings.attributes[VertexAttributeType::TANGENT],
                                                settings.attributes[VertexAttributeType::UV],
                                                settings.attributes[VertexAttributeType::COLOR]);
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

    // Viewport
    VkViewport viewport = Init::viewport(extent);
    VkRect2D   scissor;
    scissor.offset = {0, 0};
    scissor.extent = extent;
    // Viewport setup (JUST ONE FOR NOW)
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext                             = nullptr;
    viewportState.viewportCount                     = 1;
    viewportState.pViewports                        = &viewport;
    viewportState.scissorCount                      = 1;
    viewportState.pScissors                         = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer =
        Init::rasterization_state_create_info(settings.poligonMode, settings.cullMode, settings.drawOrder);

    // Depth Setup
    VkPipelineDepthStencilStateCreateInfo depthStencil =
        Init::depth_stencil_create_info(settings.depthTest ? VK_TRUE : VK_FALSE,
                                        settings.depthWrite ? VK_TRUE : VK_FALSE,
                                        settings.depthTest ? settings.depthOp : VK_COMPARE_OP_ALWAYS);

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = Init::multisampling_state_create_info(settings.samples);
    multisampling.sampleShadingEnable =
        settings.samples > VK_SAMPLE_COUNT_1_BIT && settings.sampleShading ? VK_TRUE : VK_FALSE;
    multisampling.minSampleShading = .2f;

    // Blending
    VkPipelineColorBlendStateCreateInfo colorBlending = Init::color_blend_create_info();
    colorBlending.attachmentCount                     = static_cast<uint32_t>(settings.blendAttachments.size());
    colorBlending.pAttachments                        = settings.blendAttachments.data();

    // Dynamic states
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(settings.dynamicStates.size());
    dynamicState.pDynamicStates    = settings.dynamicStates.data();

    // build the actual pipeline
    // we now use all of the info structs we have been writing into into this one
    // to create the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext                        = nullptr;

    pipelineInfo.stageCount          = (uint32_t)shaderStages.size();
    pipelineInfo.pStages             = shaderStages.data();
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;

    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout             = layout;
    pipelineInfo.renderPass         = renderPass;
    pipelineInfo.subpass            = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        throw VKException("Failed to create Grahic "
                          "Pipeline");
    }
}
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END