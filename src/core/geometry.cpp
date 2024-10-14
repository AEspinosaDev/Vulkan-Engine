#include <engine/core/geometry.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Geometry::fill(std::vector<utils::Vertex> vertexInfo)
{
    m_geometryData.vertexData = vertexInfo;
    m_geometryData.compute_statistics();
    m_geometryData.loaded = true;
}
void Geometry::fill(std::vector<utils::Vertex> vertexInfo, std::vector<uint32_t> vertexIndex)
{
    m_geometryData.vertexData = vertexInfo;
    m_geometryData.vertexIndex = vertexIndex;
    m_geometryData.compute_statistics();
    m_geometryData.loaded = true;
}

void Geometry::fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber)
{
    for (size_t i = 0; i < vertNumber; i++)
    {
        m_geometryData.vertexData.push_back({pos[i], normal[i], tangent[i], uv[i], Vec3(1.0)});
    }
    m_geometryData.compute_statistics();
}

void GeometryData::compute_statistics()
{
    maxCoords = {0.0f, 0.0f, 0.0f};
    minCoords = {INFINITY, INFINITY, INFINITY};

    for (const utils::Vertex &v : vertexData)
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

    center = (maxCoords + minCoords) * 0.5f;
}
VULKAN_ENGINE_NAMESPACE_END