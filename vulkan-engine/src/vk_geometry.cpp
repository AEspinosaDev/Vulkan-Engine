#include "engine/vk_geometry.h"

namespace vke
{

  void Geometry::fill(std::vector<Vertex> vertexInfo)
  {
    m_vertexData = vertexInfo;
    loaded = true;
    indexed = false;
  }
  void Geometry::fill(std::vector<Vertex> vertexInfo, std::vector<uint16_t> vertexIndex)
  {
    if (vertexIndex.empty())
    {
      fill(vertexInfo);
      return;
    }
    m_vertexData = vertexInfo;
    m_vertexIndex = vertexIndex;
    loaded = true;
    indexed = true;
  }

  void Geometry::fill(glm::vec3 *pos, glm::vec3 *normal, glm::vec2 *uv, glm::vec3 *tangent, uint32_t vertNumber)
  {
    for (size_t i = 0; i < vertNumber; i++)
    {
      m_vertexData.push_back({pos[i], normal[i], tangent[i], uv[i], glm::vec3(1.0)});
    }
  }

}