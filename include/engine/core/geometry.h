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
#include <engine/backend/buffer.h>
#include <engine/backend/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

/*
Canonical vertex definition. Most meshes will use some or all of these attributes.
*/
struct Vertex
{
    Vec3 pos;
    Vec3 normal;
    Vec3 tangent;
    Vec2 texCoord;
    Vec3 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(bool normal = true, bool tangent = true, bool texCoord = true, bool color = true)
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        VkVertexInputAttributeDescription posAtt{};
        posAtt.binding = 0;
        posAtt.location = 0;
        posAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAtt.offset = offsetof(Vertex, pos);
        attributeDescriptions.push_back(posAtt);

        if (normal)
        {
            VkVertexInputAttributeDescription normalAtt{};
            normalAtt.binding = 0;
            normalAtt.location = 1;
            normalAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            normalAtt.offset = offsetof(Vertex, normal);
            attributeDescriptions.push_back(normalAtt);
        }
        if (texCoord)
        {
            VkVertexInputAttributeDescription texCoordAtt{};
            texCoordAtt.binding = 0;
            texCoordAtt.location = 2;
            texCoordAtt.format = VK_FORMAT_R32G32_SFLOAT;
            texCoordAtt.offset = offsetof(Vertex, texCoord);
            attributeDescriptions.push_back(texCoordAtt);
        }
        if (tangent)
        {
            VkVertexInputAttributeDescription tangentAtt{};
            tangentAtt.binding = 0;
            tangentAtt.location = 3;
            tangentAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            tangentAtt.offset = offsetof(Vertex, tangent);
            attributeDescriptions.push_back(tangentAtt);
        }
        if (color)
        {
            VkVertexInputAttributeDescription colorAtt{};
            colorAtt.binding = 0;
            colorAtt.location = 4;
            colorAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            colorAtt.offset = offsetof(Vertex, color);
            attributeDescriptions.push_back(colorAtt);
        }

        return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && normal == other.normal && tangent == other.tangent && texCoord == other.texCoord && color == other.color;
    }

    bool operator!=(const Vertex &other) const
    {
        return !(*this == other);
    }
};

class Geometry;

struct GeometryStats
{
    Vec3 maxCoords;
    Vec3 minCoords;
    Vec3 center;

    void compute_statistics(Geometry *g);
};

/*
Class that defines the mesh geometry. Can be setup by filling it with a canonical vertex type array. 
*/
class Geometry
{

private:
    Buffer *m_vbo;
    Buffer *m_ibo;

    std::vector<uint16_t> m_vertexIndex;
    std::vector<Vertex> m_vertexData;

    size_t m_materialID{0};

    bool m_loaded{false};
    bool m_indexed{false};
    bool m_buffers_loaded{false};

    friend class Renderer;

public:
    Geometry() : m_vbo{new Buffer},
                 m_ibo{new Buffer} {}

    inline size_t get_material_ID() const { return m_materialID; }
    inline void set_material_ID(size_t id) { m_materialID = id; }

    inline bool is_data_loaded() const { return m_loaded; }
    inline bool is_buffer_loaded() const { return m_buffers_loaded; }
    inline bool is_indexed() const { return m_indexed; }

    inline std::vector<uint16_t> const get_vertex_index() const
    {
        return m_vertexIndex;
    }
    inline std::vector<Vertex> const get_vertex_data() const
    {
        return m_vertexData;
    }
    ~Geometry()
    {
        delete m_vbo;
        delete m_ibo;
    }

    void fill(std::vector<Vertex> vertexInfo);
    void fill(std::vector<Vertex> vertexInfo, std::vector<uint16_t> vertexIndex);
    void fill(Vec3 *pos, Vec3 *normal, Vec2 *uv, Vec3 *tangent, uint32_t vertNumber);

    //Utility function for drawing purposes
    static void draw(VkCommandBuffer &cmd, Geometry *const g);
};

VULKAN_ENGINE_NAMESPACE_END;

namespace std
{
    template <>
    struct hash<vke::Vertex>
    {
        size_t operator()(vke::Vertex const &vertex) const
        {
            size_t seed = 0;
            vke::utils::hash_combine(seed, vertex.pos, vertex.normal, vertex.tangent, vertex.texCoord, vertex.color);
            return seed;
        }
    };
};

#endif // VK_GEOMETRY_H