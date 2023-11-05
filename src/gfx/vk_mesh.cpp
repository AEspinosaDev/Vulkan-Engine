#include "vk_mesh.h"

namespace vkeng {

	void Mesh::cache_buffer(VkDevice device, VkPhysicalDevice gpu)
	{

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(m_vertexData[0]) * m_vertexData.size();
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_vbo) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, m_vbo, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = vkutils::find_memory_type(gpu, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &m_memory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		VK_CHECK(vkBindBufferMemory(device, m_vbo, m_memory, 0));

		void* data;
		vkMapMemory(device, m_memory, 0, bufferInfo.size, 0, &data);
		memcpy(data, m_vertexData.data(), (size_t)bufferInfo.size);
		vkUnmapMemory(device, m_memory);

		buffer_loaded = true;
	}


	Mesh* Mesh::load()
	{
		Mesh* m = new Mesh();
		m->m_vertexData = {
		{{0.0f, -0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {0.0f, 0.0f, 1.0f}}
		};
		/*m->m_vertexData = {
		{{0.0f, -0.5f,0.0f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f,0.0f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f,0.0f}, {0.0f, 0.0f, 1.0f}}
		};*/

		m->loaded = true;
		return m;

	}
	Mesh* Mesh::load2()
	{
		Mesh* m = new Mesh();
		m->m_vertexData = {
		{{0.1f, -0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {1.0f, 1.0f, 1.0f}},
		{{0.6f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {1.0f, 0.0f, 0.0f}},
		{{-0.4f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {0.0f, 1.0f, 1.0f}}
		};
		/*m->m_vertexData = {
		{{0.0f, -0.5f,0.0f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f,0.0f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f,0.0f}, {0.0f, 0.0f, 1.0f}}
		};*/

		m->loaded = true;
		return m;

	}
	void  Mesh::cleanup_buffer(VkDevice device) {
		vkDestroyBuffer(device, m_vbo, nullptr);
		vkFreeMemory(device, m_memory, nullptr);
	}

}