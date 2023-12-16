#include "vk_pipeline.h"

namespace vke
{

	VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
	{
		// make viewport state from our stored viewport and scissor.
		// at the moment we wont support multiple viewports or scissors
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// setup dummy color blending. We arent using transparent objects yet
		// the blending is just "no blend", but we do write to the color attachment
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;

		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		// build the actual pipeline
		// we now use all of the info structs we have been writing into into this one to create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;

		std::vector<VkPipelineShaderStageCreateInfo> stages;
		for (auto &stage : shaderPass->stages)
		{
			stages.push_back(vkinit::pipeline_shader_stage_create_info(stage.stage, stage.shaderModule));
		}

		pipelineInfo.stageCount = stages.size();
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
		pipelineInfo.renderPass = pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkPipeline newPipeline;
		if (vkCreateGraphicsPipelines(
				device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
		{
			std::cout << "failed to create pipline\n";
			return VK_NULL_HANDLE; // failed to create graphics pipeline
		}
		else
		{
			return newPipeline;
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

	void ShaderPass::cleanup(VkDevice device)
	{
		for (auto &stage : stages)
		{
			vkDestroyShaderModule(device, stage.shaderModule, nullptr);
		}
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
	}

}