#pragma once
#include "backend/vk_core.h"
#include "backend/vk_utils.h"

//struct for buffers
//struct BufferData {
//
//	unsigned int triangles;
//	unsigned int numFaces{ 0 };
//	unsigned int numVertices{ 0 };
//	unsigned int* faceArray{ nullptr };
//
//	float* positions{ nullptr };
//	float* normals{ nullptr };
//	float* colors{ nullptr };
//	float* texCoords{ nullptr };
//	float* tangents{ nullptr };
//
//
//};
namespace vkeng {

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

			VkVertexInputAttributeDescription posAtt{};
			posAtt.binding = 0;
			posAtt.location = 0;
			posAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
			posAtt.offset = offsetof(Vertex, pos);
			attributeDescriptions.push_back(posAtt);
			VkVertexInputAttributeDescription normalAtt{};
			normalAtt.binding = 0;
			normalAtt.location = 1;
			normalAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
			normalAtt.offset = offsetof(Vertex, normal);
			attributeDescriptions.push_back(normalAtt);
			VkVertexInputAttributeDescription texCoordAtt{};
			texCoordAtt.binding = 0;
			texCoordAtt.location = 2;
			texCoordAtt.format = VK_FORMAT_R32G32_SFLOAT;
			texCoordAtt.offset = offsetof(Vertex, texCoord);
			attributeDescriptions.push_back(texCoordAtt);
			VkVertexInputAttributeDescription tangentAtt{};
			tangentAtt.binding = 0;
			tangentAtt.location = 3;
			tangentAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
			tangentAtt.offset = offsetof(Vertex, tangent);
			attributeDescriptions.push_back(tangentAtt);
			VkVertexInputAttributeDescription colorAtt{};
			colorAtt.binding = 0;
			colorAtt.location = 4;
			colorAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
			colorAtt.offset = offsetof(Vertex, color);
			attributeDescriptions.push_back(colorAtt);

			return attributeDescriptions;
		}
	};

	struct VertexBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;

	};


	class Mesh {

	private:
		std::vector<Vertex>  m_vertexData;
		VkBuffer	m_vbo;
		VkDeviceMemory m_memory;
		bool loaded{ false };
		bool buffer_loaded{ false };

	public:
		Mesh() : m_vbo{}, m_memory{} {}
		inline bool is_data_loaded() { return loaded; }
		inline bool is_buffer_loaded() { return buffer_loaded; }
		inline VkBuffer const get_vbo() const { return m_vbo; }
		inline std::vector<Vertex> const get_vertex_data() const {
			return m_vertexData;
		}

		void cache_buffer(VkDevice device, VkPhysicalDevice gpu);
		void cleanup_buffer(VkDevice device);
		static Mesh* load_file();
		static Mesh* load();
		static Mesh* load2();

		
	};

}