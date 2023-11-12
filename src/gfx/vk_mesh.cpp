#include "vk_mesh.h"

namespace vkeng {
	
    Mesh *Mesh::load(std::vector<Vertex> vertexInfo)
    {
        Mesh* m = new Mesh();
		m->m_vertexData = vertexInfo;
		m->loaded = true;
		m->indexed = false;
		return m;
    }
    Mesh *Mesh::load(std::vector<Vertex> vertexInfo, std::vector<uint16_t> vertexIndex)
    {
         Mesh* m = new Mesh();
		m->m_vertexData = vertexInfo;
		m->m_vertexIndex = vertexIndex;
		m->loaded = true;
		m->indexed = true;
		return m;
    }
    Mesh *Mesh::load(glm::vec3 **pos, glm::vec3 **normal, glm::vec2 **uv, glm::vec3 **tangent)
    {
		
        return nullptr;
    }
    Mesh *Mesh::load2()
    {
		Mesh* m = new Mesh();
		m->m_vertexData = {
		{{0.1f, -0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {1.0f, 1.0f, 1.0f}},
		{{0.6f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {1.0f, 0.0f, 0.0f}},
		{{-0.4f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {0.0f, 1.0f, 1.0f}}
		};
		
		m->loaded = true;
		return m;

	}
	
}