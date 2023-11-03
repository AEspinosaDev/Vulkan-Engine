#pragma once
#include "vk_core.h"
/*
/Shader useful info
*/
struct Shader
{
	std::string								name;

	std::string								vertSource;
	std::string								fragSource;
	std::string								geomSource;
	std::string								tessSource;

	static VkShaderModule create_shader_module(VkDevice device, const std::vector<uint32_t> code);
	static Shader read_file(const std::string& filePath);
	static std::vector<uint32_t> compile_shader(const std::string src, const std::string shaderName, shaderc_shader_kind kind, bool optimize);
};

/*
/Pipeline data and creation wrapper
*/
struct PipelineBuilder {

	VkViewport viewport;
	VkRect2D scissor;

	//Shaders
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	//Vertex attributes
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	//Primitive type
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	//Poligon mode, culling and order
	VkPipelineRasterizationStateCreateInfo rasterizer;
	//Blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	//Sampling
	VkPipelineMultisampleStateCreateInfo multisampling;

	//Maybe a map containing pipelines and layouts?
	VkPipelineLayout pipelineLayout;

	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);


};


