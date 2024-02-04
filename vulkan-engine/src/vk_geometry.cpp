#include <engine/vk_geometry.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

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

void Geometry::fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber)
{
  for (size_t i = 0; i < vertNumber; i++)
  {
    m_vertexData.push_back({pos[i], normal[i], tangent[i], uv[i], Vec3(1.0)});
  }
}

VULKAN_ENGINE_NAMESPACE_END