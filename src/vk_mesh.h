#pragma once
#include "vk_core.h"

//struct for buffers
struct BufferData {

	unsigned int triangles;
	unsigned int numFaces{ 0 };
	unsigned int numVertices{ 0 };
	unsigned int* faceArray{ nullptr };

	float* positions{ nullptr };
	float* normals{ nullptr };
	float* colors{ nullptr };
	float* texCoords{ nullptr };
	float* tangents{ nullptr };

	
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 texCoord;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		VkVertexInputAttributeDescription att0{};
		att0.binding = 0;
		att0.location = 0;
		att0.format = VK_FORMAT_R32G32B32_SFLOAT;
		att0.offset = offsetof(Vertex, pos);
		VkVertexInputAttributeDescription att1{};
		att1.binding = 0;
		att1.location = 1;
		att1.format = VK_FORMAT_R32G32B32_SFLOAT;
		att1.offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
};


class Mesh {
	
private:
	//VkPipeline* m_pipeline;
	BufferData  m_tmpBufferData;

public:
	Mesh(){}
	//Mesh() :m_pipeline(nullptr) {}

	static void load_file();
	static void cache_buffer();
	static Mesh* load();
};