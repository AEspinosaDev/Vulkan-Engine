#include <engine/backend/pipeline.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void PipelineBuilder::init(VkExtent2D &extent)
{

	// Default geometry assembly values
	vertexInputInfo = init::vertex_input_state_create_info();
	inputAssembly = init::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	auto bindingDescription = Vertex::getBindingDescription();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	// Viewport
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	scissor.offset = {0, 0};
	scissor.extent = extent;

	rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

	depthStencil = init::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS);
}

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

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pass.pipelineLayout) != VK_SUCCESS)
	{
		throw new VKException("failed to create pipeline layout!");
	}

	pipelineLayout = pass.pipelineLayout;
}

void PipelineBuilder::build_pipeline(VkDevice &device, VkRenderPass renderPass, ShaderPass &shaderPass)
{
	// Viewport setup (JUST ONE FOR NOW)
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Attribute setup
	auto attributeDescriptions = Vertex::getAttributeDescriptions(shaderPass.settings.attributes[VertexAttributeType::NORMAL],
																  shaderPass.settings.attributes[VertexAttributeType::TANGENT],
																  shaderPass.settings.attributes[VertexAttributeType::UV],
																  shaderPass.settings.attributes[VertexAttributeType::COLOR]);
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Raster setup
	rasterizer.polygonMode = shaderPass.settings.poligonMode;
	rasterizer.cullMode = shaderPass.settings.cullMode;
	rasterizer.frontFace = shaderPass.settings.drawOrder;

	// Depth setup
	depthStencil.depthTestEnable = shaderPass.settings.depthTest ? VK_TRUE : VK_FALSE;
	depthStencil.depthWriteEnable = shaderPass.settings.depthWrite ? VK_TRUE : VK_FALSE;
	depthStencil.depthCompareOp = shaderPass.settings.depthTest ? shaderPass.settings.depthOp : VK_COMPARE_OP_ALWAYS;

	// Blending SETUP TO DO
	if (!shaderPass.settings.blending)
	{
		colorBlendAttachment = init::color_blend_attachment_state();
		colorBlending = init::color_blend_create_info();
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
	}
	else
	{
		//TO DO: BLENDING
	}

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

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
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(
			device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
	{
		throw VKException("Failed to create Grahic Pipeline")
	}
	else
	{
		shaderPass.pipeline = newPipeline;
	}
}

ShaderStage ShaderSource::create_shader_stage(VkDevice device, VkShaderStageFlagBits stageType, const std::vector<uint32_t> code)
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

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1,
		GEOMETRY = 2,
		TESSELATION = 3
	};

	std::string line;
	std::stringstream ss[4];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
			else if (line.find("geometry") != std::string::npos)
				type = ShaderType::GEOMETRY;
			else if (line.find("tesselation") != std::string::npos)
				type = ShaderType::TESSELATION;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}
	return {filePath, ss[0].str(), ss[1].str(), ss[2].str(), ss[3].str()};
}

std::vector<uint32_t> ShaderSource::compile_shader(const std::string src, const std::string shaderName, shaderc_shader_kind kind, bool optimize)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	if (optimize)
	{
		options.SetOptimizationLevel(shaderc_optimization_level_size);
	}
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(src, kind, shaderName.c_str(), options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		DEBUG_LOG("Error compiling module - " << result.GetErrorMessage());
	}

	std::vector<uint32_t> spirv = {result.cbegin(), result.cend()};

	return spirv;
}

void ShaderPass::build_shader_stages(VkDevice &device, ShaderPass &pass)
{
	if (pass.SHADER_FILE == "")
		return;
	auto shader = ShaderSource::read_file(pass.SHADER_FILE);

	if (shader.vertSource != "")
	{
		ShaderStage vertShaderStage = ShaderSource::create_shader_stage(device, VK_SHADER_STAGE_VERTEX_BIT,
																		ShaderSource::compile_shader(shader.vertSource, shader.name + "vert", shaderc_vertex_shader, true));
		pass.stages.push_back(vertShaderStage);
	}
	if (shader.fragSource != "")
	{
		ShaderStage fragShaderStage = ShaderSource::create_shader_stage(device, VK_SHADER_STAGE_FRAGMENT_BIT,
																		ShaderSource::compile_shader(shader.fragSource, shader.name + "frag", shaderc_fragment_shader, true));
		pass.stages.push_back(fragShaderStage);
	}
	if (shader.geomSource != "")
	{
		ShaderStage geomShaderStage = ShaderSource::create_shader_stage(device, VK_SHADER_STAGE_GEOMETRY_BIT,
																		ShaderSource::compile_shader(shader.geomSource, shader.name + "geom", shaderc_geometry_shader, true));
		pass.stages.push_back(geomShaderStage);
	}
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

VULKAN_ENGINE_NAMESPACE_END