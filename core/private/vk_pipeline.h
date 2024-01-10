#ifndef VK_PIPELINE
#define VK_PIPELINE

#include "vk_core.h"
#include "vk_initializers.h"

namespace vke
{
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

	struct ShaderPass
	{

		const std::string SHADER_FILE;

		VkPipeline pipeline{VK_NULL_HANDLE};
		VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

		std::vector<ShaderStage> stages;
		std::vector<int> descriptorSetLayoutIDs;

		ShaderPass(const std::string shaderFile) : SHADER_FILE(shaderFile) {}

		void cleanup(VkDevice device);
	};

	/*
	/Pipeline data and creation wrapper
	*/
	struct PipelineBuilder
	{

		VkViewport viewport;
		VkRect2D scissor;

		// ShadersPass
		ShaderPass *shaderPass;
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
			VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineLayout pipelineLayout;

		VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
	};

}
#endif