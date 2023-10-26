#pragma once
#include "vk_core.h"


//Maybe a shader compiling queue?

struct ShaderSource
{
	std::string								vert;
	std::string								frag;
	std::string								geom;
	std::string								tess;
};

class Shader {
	
	const std::string& m_name;
	ShaderSource m_source;


	ShaderSource readFile(const std::string& filePath);
public:
	Shader(const std::string& fileName): m_name(fileName){

		m_source = readFile(fileName);
	}

	inline ShaderSource* getSource() {
		return &m_source;
	}

	static VkShaderModule createShaderModule(VkDevice device, const std::vector<uint32_t> code);
	static std::vector<uint32_t> compileShader(const std::string src, const std::string shaderName, shaderc_shader_kind kind, bool optimize);


};