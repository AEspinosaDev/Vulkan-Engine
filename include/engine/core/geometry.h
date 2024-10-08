/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/

#ifndef GEOMETRY_H
#define GEOMETRY_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <engine/common.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN



class Geometry;

struct GeometryStats
{
    Vec3 maxCoords;
    Vec3 minCoords;
    Vec3 center;

    void compute_statistics(Geometry *g);
};

struct RenderData
{
    bool loaded{false};
    Buffer vbo{};
    Buffer ibo{};
};

/*
Class that defines the mesh geometry. Can be setup by filling it with a canonical vertex type array.
*/
class Geometry
{

private:
    RenderData m_renderData{};

    std::vector<uint16_t> m_vertexIndex;
    std::vector<utils::Vertex> m_vertexData;

    size_t m_materialID{0};

    bool m_loaded{false};
    bool m_indexed{false};

public:
    Geometry() {}

    inline size_t get_material_ID() const { return m_materialID; }
    inline void set_material_ID(size_t id) { m_materialID = id; }

    inline bool data_loaded() const { return m_loaded; }
    inline bool indexed() const { return m_indexed; }

    inline std::vector<uint16_t> const get_vertex_index() const { return m_vertexIndex; }
    inline std::vector<utils::Vertex> const get_vertex_data() const { return m_vertexData; }
    inline RenderData get_render_data() const { return m_renderData; }
    inline void set_render_data(RenderData rd) { m_renderData = rd; }

    ~Geometry() {}

    void fill(std::vector<utils::Vertex> vertexInfo);
    void fill(std::vector<utils::Vertex> vertexInfo, std::vector<uint16_t> vertexIndex);
    void fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber);

};

VULKAN_ENGINE_NAMESPACE_END;

namespace std
{
    template <>
    struct hash<VkFW::utils::Vertex>
    {
        size_t operator()(VkFW::utils::Vertex const &vertex) const
        {
            size_t seed = 0;
            VkFW::utils::hash_combine(seed, vertex.pos, vertex.normal, vertex.tangent, vertex.texCoord, vertex.color);
            return seed;
        }
    };
};

#endif // VK_GEOMETRY_H