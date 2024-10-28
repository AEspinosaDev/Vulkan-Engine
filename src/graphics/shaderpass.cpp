#include <engine/graphics/shaderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

void PipelineBuilder::build_pipeline_layout(VkDevice &device, DescriptorManager &descriptorManager, ShaderPass &pass)
{
    std::vector<VkDescriptorSetLayout> descriptorLayouts;
    for (auto &layoutID : pass.settings.descriptorSetLayoutIDs)
    {
        if (layoutID.second)
            descriptorLayouts.push_back(descriptorManager.get_layout(layoutID.first));
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = init::pipeline_layout_create_info();
    pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

    if (!pass.settings.pushConstants.empty())
    {
        pipelineLayoutInfo.pushConstantRangeCount = pass.settings.pushConstants.size();
        pipelineLayoutInfo.pPushConstantRanges = pass.settings.pushConstants.data();
    }

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pass.pipelineLayout) != VK_SUCCESS)
    {
        throw new VKException("failed to create pipeline layout!");
    }
}
void PipelineBuilder::build_pipeline(VkDevice &device, VkRenderPass renderPass, VkExtent2D &extent,
                                     ShaderPass &shaderPass)
{
    // Vertex and geometry
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = init::vertex_input_state_create_info();
    VkPipelineInputAssemblyStateCreateInfo inputAssembly =
        init::input_assembly_create_info(shaderPass.settings.topology);

    auto bindingDescription = utils::Vertex::getBindingDescription();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

    auto attributeDescriptions =
        utils::Vertex::getAttributeDescriptions(shaderPass.settings.attributes[VertexAttributeType::POSITION],
                                                shaderPass.settings.attributes[VertexAttributeType::NORMAL],
                                                shaderPass.settings.attributes[VertexAttributeType::TANGENT],
                                                shaderPass.settings.attributes[VertexAttributeType::UV],
                                                shaderPass.settings.attributes[VertexAttributeType::COLOR]);
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Viewport
    VkViewport viewport = init::viewport(extent);
    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = extent;
    // Viewport setup (JUST ONE FOR NOW)
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = init::rasterization_state_create_info(
        shaderPass.settings.poligonMode, shaderPass.settings.cullMode, shaderPass.settings.drawOrder);

    // Depth Setup
    VkPipelineDepthStencilStateCreateInfo depthStencil = init::depth_stencil_create_info(
        shaderPass.settings.depthTest ? VK_TRUE : VK_FALSE, shaderPass.settings.depthWrite ? VK_TRUE : VK_FALSE,
        shaderPass.settings.depthTest ? shaderPass.settings.depthOp : VK_COMPARE_OP_ALWAYS);

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling =
        init::multisampling_state_create_info(shaderPass.settings.samples);

    // Blending
    VkPipelineColorBlendStateCreateInfo colorBlending = init::color_blend_create_info();
    colorBlending.attachmentCount = static_cast<uint32_t>(shaderPass.settings.blendAttachments.size());
    colorBlending.pAttachments = shaderPass.settings.blendAttachments.data();

    // Dynamic states
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(shaderPass.settings.dynamicStates.size());
    dynamicState.pDynamicStates = shaderPass.settings.dynamicStates.data();

    // build the actual pipeline
    // we now use all of the info structs we have been writing into into this one to create the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> stages;
    for (auto &stage : shaderPass.stages)
    {
        stages.push_back(init::pipeline_shader_stage_create_info(stage.stage, stage.shaderModule));
    }

    pipelineInfo.stageCount = (uint32_t)stages.size();
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = shaderPass.pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline newPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
    {
        throw VKException("Failed to create Grahic Pipeline");
    }
    else
    {
        shaderPass.pipeline = newPipeline;
    }
}

ShaderStage ShaderSource::create_shader_stage(VkDevice device, VkShaderStageFlagBits stageType,
                                              const std::vector<uint32_t> code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(unsigned int);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    ShaderStage stage;
    stage.shaderModule = shaderModule;
    stage.stage = stageType;

    return stage;
}

ShaderSource ShaderSource::read_file(const std::string &filePath)
{

    std::ifstream stream(filePath);
    std::string scriptsPath(ENGINE_RESOURCES_PATH "shaders/scripts/");

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
        TESS_CONTROL = 3,
        TESS_EVALUATION = 4
    };

    std::string line;
    std::stringstream ss[4];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        // SELECT THE SHADER STAGE
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
            else if (line.find("geometry") != std::string::npos)
                type = ShaderType::GEOMETRY;
            else if (line.find("tesscontrol") != std::string::npos)
                type = ShaderType::TESS_CONTROL;
            else if (line.find("tesseval") != std::string::npos)
                type = ShaderType::TESS_EVALUATION;
        }
        // CHECK MODULES INCLUDED
        else if (line.find("#include") != std::string::npos)
        {
            size_t start = line.find(" ") + 1;
            std::string includeFile = utils::trim(line.substr(start)); // Extract the file name and trim any spaces

            std::string fullIncludePath = scriptsPath + includeFile;

            std::string includeContent = utils::read_file(fullIncludePath);
            ss[(int)type] << includeContent << '\n';
        }
        // FILL THE ACTUAL STAGE CODE
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return {filePath, ss[0].str(), ss[1].str(), ss[2].str(), ss[3].str()};
}

std::vector<uint32_t> ShaderSource::compile_shader(const std::string src, const std::string shaderName,
                                                   shaderc_shader_kind kind, shaderc_optimization_level optimization)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetOptimizationLevel(optimization);

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(src, kind, shaderName.c_str(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        DEBUG_LOG("Error compiling module - " << result.GetErrorMessage());
    }

    std::vector<uint32_t> spirv = {result.cbegin(), result.cend()};

    return spirv;
}

void ShaderPass::build_shader_stages(VkDevice &device, ShaderPass &pass, shaderc_optimization_level optimization)
{
    if (pass.SHADER_FILE == "")
        return;
    auto shader = ShaderSource::read_file(pass.SHADER_FILE);

    if (shader.vertSource != "")
    {
        ShaderStage vertShaderStage = ShaderSource::create_shader_stage(
            device, VK_SHADER_STAGE_VERTEX_BIT,
            ShaderSource::compile_shader(shader.vertSource, shader.name + "vert", shaderc_vertex_shader, optimization));
        pass.stages.push_back(vertShaderStage);
    }
    if (shader.fragSource != "")
    {
        ShaderStage fragShaderStage =
            ShaderSource::create_shader_stage(device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                              ShaderSource::compile_shader(shader.fragSource, shader.name + "frag",
                                                                           shaderc_fragment_shader, optimization));
        pass.stages.push_back(fragShaderStage);
    }
    if (shader.geomSource != "")
    {
        ShaderStage geomShaderStage =
            ShaderSource::create_shader_stage(device, VK_SHADER_STAGE_GEOMETRY_BIT,
                                              ShaderSource::compile_shader(shader.geomSource, shader.name + "geom",
                                                                           shaderc_geometry_shader, optimization));
        pass.stages.push_back(geomShaderStage);
    }
    if (shader.tessControlSource != "")
    {
        ShaderStage tessControlShaderStage = ShaderSource::create_shader_stage(
            device, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            ShaderSource::compile_shader(shader.tessControlSource, shader.name + "control", shaderc_tess_control_shader,
                                         optimization));
        pass.stages.push_back(tessControlShaderStage);
    }
    if (shader.tessEvalSource != "")
    {
        ShaderStage tessEvalShaderStage = ShaderSource::create_shader_stage(
            device, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            ShaderSource::compile_shader(shader.tessEvalSource, shader.name + "eval", shaderc_tess_evaluation_shader,
                                         optimization));
        pass.stages.push_back(tessEvalShaderStage);
    }
}

void ShaderPass::build(VkDevice &device, VkRenderPass renderPass, DescriptorManager &descriptorManager,
                       VkExtent2D &extent, ShaderPass &shaderPass)
{
    PipelineBuilder::build_pipeline_layout(device, descriptorManager, shaderPass);
    PipelineBuilder::build_pipeline(device, renderPass, extent, shaderPass);
}
void ShaderPass::cleanup(VkDevice &device)
{
    for (auto &stage : stages)
    {
        vkDestroyShaderModule(device, stage.shaderModule, nullptr);
    }
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END