#include "vk_mesh.h"

void Mesh::cache_buffer()
{
	/*VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}*/
}


Mesh* Mesh::load()
{
	Mesh* m = new Mesh();
	//vertex positions
	m->m_tmpBufferData.positions = new float[1.f, 1.f, 0.0f,
		-1.f, 1.f, 0.0f,
		0.f, -1.f, 0.0f];
	m->m_tmpBufferData.colors = new float[0.f, 1.f, 0.0f,
		0.f, 1.f, 0.0f,
		0.f, 1.f, 0.0f];


	return m;

}
