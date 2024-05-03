#include <engine/core/geometry.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Geometry::fill(std::vector<Vertex> vertexInfo)
{
  m_vertexData = vertexInfo;
  m_loaded = true;
  m_indexed = false;
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
  m_loaded = true;
  m_indexed = true;
}

void Geometry::fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber)
{
  for (size_t i = 0; i < vertNumber; i++)
  {
    m_vertexData.push_back({pos[i], normal[i], tangent[i], uv[i], Vec3(1.0)});
  }
}

void Geometry::draw(VkCommandBuffer &cmd, Geometry *const g)
{
  VkBuffer vertexBuffers[] = {g->m_vbo->buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

  if (g->m_indexed)
  {
    vkCmdBindIndexBuffer(cmd, g->m_ibo->buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(g->m_vertexIndex.size()), 1, 0, 0, 0);
  }
  else
  {
    vkCmdDraw(cmd, static_cast<uint32_t>(g->m_vertexData.size()), 1, 0, 0);
  }
}

void GeometryStats::compute_statistics(Geometry *g)
{
  maxCoords = {0.0f, 0.0f, 0.0f};
  minCoords = {INFINITY, INFINITY, INFINITY};

  for (const Vertex &v : g->get_vertex_data())
  {
    if (v.pos.x > maxCoords.x)
      maxCoords.x = v.pos.x;
    if (v.pos.y > maxCoords.y)
      maxCoords.y = v.pos.y;
    if (v.pos.z > maxCoords.z)
      maxCoords.z = v.pos.z;
    if (v.pos.x < minCoords.x)
      minCoords.x = v.pos.x;
    if (v.pos.y < minCoords.y)
      minCoords.y = v.pos.y;
    if (v.pos.z < minCoords.z)
      minCoords.z = v.pos.z;
  }

  center = (maxCoords - minCoords) * 0.5f;
}
VULKAN_ENGINE_NAMESPACE_END