#include "vk_shader.h"

//Shader::ROOT_PATH = "../resources/shaders/";

ShaderSource Shader::readFile(const std::string& filePath)
{
	std::ifstream stream(filePath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2, TESSELATION = 3
	};

	std::string line;
	std::stringstream ss[4];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos) type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos) type = ShaderType::FRAGMENT;
			else if (line.find("geometry") != std::string::npos) type = ShaderType::GEOMETRY;
			else if (line.find("tesselation") != std::string::npos) type = ShaderType::TESSELATION;
		}
		else {
			ss[(int)type] << line << '\n';
		}
	}
	return { ss[0].str(), ss[1].str(), ss[2].str(),ss[3].str() };
}

VkShaderModule Shader::createShaderModule(VkDevice device,const std::vector<uint32_t> code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(unsigned int);
	createInfo.pCode = code.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

std::vector<uint32_t> Shader::compileShader(const std::string src, const std::string shaderName, shaderc_shader_kind kind, bool optimize)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	if (optimize) {
		options.SetOptimizationLevel(shaderc_optimization_level_size);
	}
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(src, kind, shaderName.c_str(), options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		DEBUG_LOG("Error compiling module - " << result.GetErrorMessage());
	}

	std::vector<uint32_t> spirv = { result.cbegin(),result.cend() };

	return spirv;

}
