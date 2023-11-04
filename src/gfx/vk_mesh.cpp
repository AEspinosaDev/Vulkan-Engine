#include "vk_mesh.h"

namespace VKENG {

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
	}


	Mesh* Mesh::load()
	{
		Mesh* m = new Mesh();
		//vertex positions
		/*m->m_vertexData.positions = new float[1.f, 1.f, 0.0f,
			-1.f, 1.f, 0.0f,
			0.f, -1.f, 0.0f];
		m->m_vertexData.colors = new float[0.f, 1.f, 0.0f,
			0.f, 1.f, 0.0f,
			0.f, 1.f, 0.0f];*/
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

		return m;

	}
	void  Mesh::cleanup_buffer(VkDevice device) {
		vkDestroyBuffer(device, m_vbo, nullptr);
		vkFreeMemory(device, m_memory, nullptr);
	}

	void Mesh::draw(VkCommandBuffer cmd) {

		VkBuffer vertexBuffers[] = { m_vbo };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(cmd, static_cast<uint32_t>(m_vertexData.size()), 1, 0, 0);
	}

}