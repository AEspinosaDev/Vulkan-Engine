/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef PIPELINE_H
#define PIPELINE_H

#include <engine/common.h>
#include <engine/graphics/initializers.h>
#include <engine/graphics/descriptors.h>
#include <engine/core/geometry.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

struct ShaderStage
{
	VkShaderModule shaderModule;
	VkShaderStageFlagBits stage;
};
/*
/Shader useful info
*/
struct ShaderSource
{
	std::string name;

	std::string vertSource;
	std::string fragSource;
	std::string geomSource;
	std::string tessSource;

	static ShaderSource read_file(const std::string &filePath);

	static std::vector<uint32_t> compile_shader(const std::string src, const std::string shaderName, shaderc_shader_kind kind, bool optimize);

	static ShaderStage create_shader_stage(VkDevice device, VkShaderStageFlagBits stageType, const std::vector<uint32_t> code);
};

struct ShaderPassSettings
{
	// Input attributes read
	std::unordered_map<int, bool> attributes;

	//Geometry type
	VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

	// Descriptor layouts state
	std::unordered_map<int, bool> descriptorSetLayoutIDs;

	// Rasterizer
	VkPolygonMode poligonMode{VK_POLYGON_MODE_FILL};
	VkCullModeFlagBits cullMode{VK_CULL_MODE_NONE};
	VkFrontFace drawOrder{VK_FRONT_FACE_CLOCKWISE};

	//Samples per pixel
	VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

	// Blending
	bool blending{false};
	std::vector<VkPipelineColorBlendAttachmentState> blendAttachments{init::color_blend_attachment_state(blending)};
	// blendingOperation{};

	// Depth Test
	bool depthTest{true};
	bool depthWrite{true};
	VkCompareOp depthOp{VK_COMPARE_OP_LESS_OR_EQUAL};

	//Dynamic states
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	
};

struct ShaderPass
{

	const std::string SHADER_FILE;

	VkPipeline pipeline{VK_NULL_HANDLE};
	VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

	std::vector<ShaderStage> stages;

	ShaderPassSettings settings{};

	ShaderPass(const std::string shaderFile) : SHADER_FILE(shaderFile) {}
	ShaderPass(const std::string shaderFile, ShaderPassSettings sett) : SHADER_FILE(shaderFile), settings(sett) {}

	static void build_shader_stages(VkDevice &device, ShaderPass &pass);
	

	void cleanup(VkDevice &device);
};

/*
/Pipeline data and creation wrapper
*/
struct PipelineBuilder
{

	VkViewport viewport;
	VkRect2D scissor;

	// Vertex attributes
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	// Primitive type
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	// Poligon mode, culling and order
	VkPipelineRasterizationStateCreateInfo rasterizer;
	// Blending
	VkPipelineColorBlendStateCreateInfo colorBlending;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	// Sampling
	VkPipelineMultisampleStateCreateInfo multisampling;
	// Depth
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	// Dynamic States
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineLayout pipelineLayout;

	void init(VkExtent2D &extent);

	void build_pipeline_layout(VkDevice &device, DescriptorManager &descriptorManager, ShaderPass &pass);

	void build_pipeline(VkDevice &device, VkRenderPass renderPass, VkExtent2D &extent, ShaderPass &shaderPass);
};

VULKAN_ENGINE_NAMESPACE_END
#endif