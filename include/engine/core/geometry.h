/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/

#ifndef GEOMETRY_H
#define GEOMETRY_H

#define GLM_ENABLE_EXPERIMENTAL
#include <engine/common.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/utils.h>
#include <glm/gtx/hash.hpp>

VULKAN_ENGINE_NAMESPACE_BEGIN

class Geometry;

struct GeometryData
{
    std::vector<uint32_t> vertexIndex;
    std::vector<utils::Vertex> vertexData;

    // Stats
    Vec3 maxCoords;
    Vec3 minCoords;
    Vec3 center;

    bool loaded{false};

    void compute_statistics();
};

struct RenderData
{
    bool loadedOnGPU{false};

    Buffer vbo{};
    Buffer ibo{};

    uint32_t vertexCount{0};
    uint32_t indexCount{0};
};

/*
Class that defines the mesh geometry. Can be setup by filling it with a canonical vertex type array.
*/
class Geometry
{

  private:
    RenderData m_renderData{};
    GeometryData m_geometryData{};

    size_t m_materialID{0};

  public:
    Geometry()
    {
    }

    inline size_t get_material_ID() const
    {
        return m_materialID;
    }
    inline void set_material_ID(size_t id)
    {
        m_materialID = id;
    }

    inline bool data_loaded() const
    {
        return m_geometryData.loaded;
    }
    inline bool indexed() const
    {
        return !m_geometryData.vertexIndex.empty();
    }

    inline GeometryData get_geometric_data()
    {
        return m_geometryData;
    }
    inline void set_geometric_data(GeometryData gd)
    {
        m_geometryData = gd;
    }

    inline RenderData get_render_data()
    {
        return m_renderData;
    }
    inline void set_render_data(RenderData rd)
    {
        m_renderData = rd;
    }

    ~Geometry()
    {
    }

    void fill(std::vector<utils::Vertex> vertexInfo);
    void fill(std::vector<utils::Vertex> vertexInfo, std::vector<uint32_t> vertexIndex);
    void fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber);
};

VULKAN_ENGINE_NAMESPACE_END;

namespace std
{
template <> struct hash<VkFW::utils::Vertex>
{
    size_t operator()(VkFW::utils::Vertex const &vertex) const
    {
        size_t seed = 0;
        VkFW::utils::hash_combine(seed, vertex.pos, vertex.normal, vertex.tangent, vertex.texCoord, vertex.color);
        return seed;
    }
};
}; // namespace std

#endif // VK_GEOMETRY_H