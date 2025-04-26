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

Geometry* Geometry::create_simple_cube() {
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
Geometry* Geometry::create_cube() {
    Geometry* g = new Geometry();

    g->fill(
        {
            // Front face (+Z)
            {{-1, -1, 1}, {}, {}, {0, 0}, {}}, // 0
            {{1, -1, 1}, {}, {}, {1, 0}, {}},  // 1
            {{1, 1, 1}, {}, {}, {1, 1}, {}},   // 2
            {{-1, 1, 1}, {}, {}, {0, 1}, {}},  // 3

            // Back face (-Z)
            {{1, -1, -1}, {}, {}, {0, 0}, {}},  // 4
            {{-1, -1, -1}, {}, {}, {1, 0}, {}}, // 5
            {{-1, 1, -1}, {}, {}, {1, 1}, {}},  // 6
            {{1, 1, -1}, {}, {}, {0, 1}, {}},   // 7

            // Left face (-X)
            {{-1, -1, -1}, {}, {}, {0, 0}, {}}, // 8
            {{-1, -1, 1}, {}, {}, {1, 0}, {}},  // 9
            {{-1, 1, 1}, {}, {}, {1, 1}, {}},   // 10
            {{-1, 1, -1}, {}, {}, {0, 1}, {}},  // 11

            // Right face (+X)
            {{1, -1, 1}, {}, {}, {0, 0}, {}},  // 12
            {{1, -1, -1}, {}, {}, {1, 0}, {}}, // 13
            {{1, 1, -1}, {}, {}, {1, 1}, {}},  // 14
            {{1, 1, 1}, {}, {}, {0, 1}, {}},   // 15

            // Top face (+Y)
            {{-1, 1, 1}, {}, {}, {0, 0}, {}},  // 16
            {{1, 1, 1}, {}, {}, {1, 0}, {}},   // 17
            {{1, 1, -1}, {}, {}, {1, 1}, {}},  // 18
            {{-1, 1, -1}, {}, {}, {0, 1}, {}}, // 19

            // Bottom face (-Y)
            {{-1, -1, -1}, {}, {}, {0, 0}, {}}, // 20
            {{1, -1, -1}, {}, {}, {1, 0}, {}},  // 21
            {{1, -1, 1}, {}, {}, {1, 1}, {}},   // 22
            {{-1, -1, 1}, {}, {}, {0, 1}, {}},  // 23
        },
        {// Front
         0,
         1,
         2,
         2,
         3,
         0,
         // Back
         4,
         5,
         6,
         6,
         7,
         4,
         // Left
         8,
         9,
         10,
         10,
         11,
         8,
         // Right
         12,
         13,
         14,
         14,
         15,
         12,
         // Top
         16,
         17,
         18,
         18,
         19,
         16,
         // Bottom
         20,
         21,
         22,
         22,
         23,
         20});

    return g;
}
Geometry* Geometry::create_cylinder(int segments, float radius, float height) {
    Geometry*                     g = new Geometry();
    std::vector<Graphics::Vertex> vertices;
    std::vector<uint32_t>         indices;

    float halfHeight = height / 2.0f;
    float angleStep  = 2.0f * 3.14159265358979323846 / segments;

    // Generate side vertices
    for (int i = 0; i <= segments; i++)
    {
        float angle = i * angleStep;
        float x     = radius * cos(angle);
        float z     = radius * sin(angle);

        vertices.push_back({{x, -halfHeight, z},
                            {0.0f, 0.0f, 0.0f},
                            {0.0f, 0.0f, 0.0f},
                            {i / (float)segments, 0.0f},
                            {0.0f, 0.0f, 0.0f}});
        vertices.push_back({{x, halfHeight, z},
                            {0.0f, 0.0f, 0.0f},
                            {0.0f, 0.0f, 0.0f},
                            {i / (float)segments, 1.0f},
                            {0.0f, 0.0f, 0.0f}});
    }

    // Side indices
    for (int i = 0; i < segments; i++)
    {
        int base = i * 2;
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }

    // Top and bottom center vertices
    int bottomCenterIdx = vertices.size();
    vertices.push_back(
        {{0.0f, -halfHeight, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}});
    int topCenterIdx = vertices.size();
    vertices.push_back(
        {{0.0f, halfHeight, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}});

    // Top and bottom cap indices
    for (int i = 0; i < segments; i++)
    {
        int base = i * 2;
        int next = ((i + 1) % segments) * 2;

        // Bottom cap
        indices.push_back(bottomCenterIdx);
        indices.push_back(base);
        indices.push_back(next);

        // Top cap
        indices.push_back(topCenterIdx);
        indices.push_back(next + 1);
        indices.push_back(base + 1);
    }

    g->fill(vertices, indices);
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

            Vec3 tangent = Utils::get_tangent_gram_smidt(vertices[i0].pos,
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
            Vec3 tangent = Utils::get_tangent_gram_smidt(vertices[i].pos,
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