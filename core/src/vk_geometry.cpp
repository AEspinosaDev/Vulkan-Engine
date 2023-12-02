#include "engine/vk_geometry.h"

namespace vke {
	
    void Geometry::fill(std::vector<Vertex> vertexInfo)
    {
		m_vertexData = vertexInfo;
		loaded = true;
		indexed = false;
    }
    void Geometry::fill(std::vector<Vertex> vertexInfo, std::vector<uint16_t> vertexIndex)
    {
		m_vertexData = vertexInfo;
		m_vertexIndex = vertexIndex;
		loaded = true;
		indexed = true;
    }
    void Geometry::fill(glm::vec3 **pos, glm::vec3 **normal, glm::vec2 **uv, glm::vec3 **tangent)
    {
		
        // return nullptr;
    }
    // Geometry *Geometry::load2()
    // {
	// 	Geometry* m = new Mesh();
	// 	m->m_vertexData = {
	// 	{{0.1f, -0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {1.0f, 1.0f, 1.0f}},
	// 	{{0.6f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {1.0f, 0.0f, 0.0f}},
	// 	{{-0.4f, 0.5f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}, {0.0f, 1.0f, 1.0f}}
	// 	};
		
	// 	m->loaded = true;
	// 	return m;

	// }
	
}