#include <engine/core/geometries/geometry.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

void Geometry::fill(std::vector<Graphics::Vertex> vertexInfo) {
    m_properties.vertexData = vertexInfo;
    m_properties.compute_statistics();
    m_properties.loaded = true;
}
void Geometry::fill(std::vector<Graphics::Vertex> vertexInfo, std::vector<uint32_t> vertexIndex) {
    m_properties.vertexData  = vertexInfo;
    m_properties.vertexIndex = vertexIndex;
    m_properties.compute_statistics();
    m_properties.loaded = true;
}

void Geometry::fill(Vec3* pos, Vec3* normal, Vec2* uv, Vec3* tangent, uint32_t vertNumber) {
    for (size_t i = 0; i < vertNumber; i++)
    {
        m_properties.vertexData.push_back({pos[i], normal[i], tangent[i], uv[i], Vec3(1.0)});
    }
    m_properties.compute_statistics();
    m_properties.loaded = true;
}

void Geometry::fill_voxel_array(std::vector<Graphics::Voxel> voxels) {
    m_properties.voxelData = voxels;
}
void GeometricData::compute_statistics() {
    maxCoords = {0.0f, 0.0f, 0.0f};
    minCoords = {INFINITY, INFINITY, INFINITY};

    for (const Graphics::Vertex& v : vertexData)
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

Geometry* Geometry::create_quad() {
    Geometry* g = new Geometry();

    g->fill({{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
             {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
             {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
             {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}}},

            {0, 1, 2, 1, 3, 2});

    return g;
}

Geometry* Geometry::create_cube() {
    Geometry* g = new Geometry();

    g->fill(
        {{{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},  // 0
         {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}, // 1
         {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},  // 2
         {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},   // 3
         {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},   // 4
         {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},  // 5
         {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},   // 6
         {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}}},   // 7
        {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 0, 3, 7, 7, 4, 0, 1, 5, 6, 6, 2, 1, 0, 1, 5, 5, 4, 0, 2, 3, 7, 7, 6, 2});

    return g;
}
void Geometry::compute_tangents_gram_smidt(std::vector<Graphics::Vertex>& vertices,
                                           const std::vector<uint32_t>&   indices) {
    if (!indices.empty())
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            size_t i0 = indices[i];
            size_t i1 = indices[i + 1];
            size_t i2 = indices[i + 2];

            Vec3 tangent = Graphics::Utils::get_tangent_gram_smidt(vertices[i0].pos,
                                                                   vertices[i1].pos,
                                                                   vertices[i2].pos,
                                                                   vertices[i0].texCoord,
                                                                   vertices[i1].texCoord,
                                                                   vertices[i2].texCoord,
                                                                   vertices[i0].normal);

            vertices[i0].tangent += tangent;
            vertices[i1].tangent += tangent;
            vertices[i2].tangent += tangent;
        }
    else
        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            Vec3 tangent = Graphics::Utils::get_tangent_gram_smidt(vertices[i].pos,
                                                                   vertices[i + 1].pos,
                                                                   vertices[i + 2].pos,
                                                                   vertices[i].texCoord,
                                                                   vertices[i + 1].texCoord,
                                                                   vertices[i + 2].texCoord,
                                                                   vertices[i].normal);

            vertices[i].tangent += tangent;
            vertices[i + 1].tangent += tangent;
            vertices[i + 2].tangent += tangent;
        }
}
Graphics::VertexArrays* const get_VAO(Geometry* g) {
    return &g->m_VAO;
}
Graphics::BLAS* const get_BLAS(Geometry* g) {

    return &g->m_BLAS;
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END