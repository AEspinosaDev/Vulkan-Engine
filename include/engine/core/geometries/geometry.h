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

namespace Core
{

class Geometry;

struct GeometricData
{
    std::vector<uint32_t> vertexIndex;
    std::vector<Graphics::utils::Vertex> vertexData;

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

    Graphics::Buffer vbo{};
    Graphics::Buffer ibo{};

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
    GeometricData m_geometryData{};

    size_t m_materialID{0};

    friend RenderData *const get_render_data(Geometry *g);

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

    inline const GeometricData *get_geometric_data() const
    {
        return &m_geometryData;
    }
    ~Geometry()
    {
    }

    void fill(std::vector<Graphics::utils::Vertex> vertexInfo);
    void fill(std::vector<Graphics::utils::Vertex> vertexInfo, std::vector<uint32_t> vertexIndex);
    void fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber);
    static Geometry *create_quad();
    static Geometry *create_cube();
};

RenderData *const get_render_data(Geometry *g);

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END;

namespace std
{
template <> struct hash<VKFW::Graphics::utils::Vertex>
{
    size_t operator()(VKFW::Graphics::utils::Vertex const &vertex) const
    {
        size_t seed = 0;
        VKFW::Graphics::utils::hash_combine(seed, vertex.pos, vertex.normal, vertex.tangent, vertex.texCoord,
                                            vertex.color);
        return seed;
    }
};
}; // namespace std

#endif // VK_GEOMETRY_H