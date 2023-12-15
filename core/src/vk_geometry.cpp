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
    
}