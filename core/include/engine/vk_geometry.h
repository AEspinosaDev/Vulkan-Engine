#ifndef VK_GEOMETRY_H
#define VK_GEOMETRY_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "../private/vk_core.h"
#include "../private/vk_buffer.h"
#include "../private/vk_utils.h"

namespace vke
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 texCoord;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return bindingDescription;
        }
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
        {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

            VkVertexInputAttributeDescription posAtt{};
            posAtt.binding = 0;
            posAtt.location = 0;
            posAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            posAtt.offset = offsetof(Vertex, pos);
            attributeDescriptions.push_back(posAtt);
            VkVertexInputAttributeDescription normalAtt{};
            normalAtt.binding = 0;
            normalAtt.location = 1;
            normalAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            normalAtt.offset = offsetof(Vertex, normal);
            attributeDescriptions.push_back(normalAtt);
            VkVertexInputAttributeDescription texCoordAtt{};
            texCoordAtt.binding = 0;
            texCoordAtt.location = 2;
            texCoordAtt.format = VK_FORMAT_R32G32_SFLOAT;
            texCoordAtt.offset = offsetof(Vertex, texCoord);
            attributeDescriptions.push_back(texCoordAtt);
            VkVertexInputAttributeDescription tangentAtt{};
            tangentAtt.binding = 0;
            tangentAtt.location = 3;
            tangentAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            tangentAtt.offset = offsetof(Vertex, tangent);
            attributeDescriptions.push_back(tangentAtt);
            VkVertexInputAttributeDescription colorAtt{};
            colorAtt.binding = 0;
            colorAtt.location = 4;
            colorAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            colorAtt.offset = offsetof(Vertex, color);
            attributeDescriptions.push_back(colorAtt);

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

    class Geometry
    {

    private:
        std::vector<uint16_t> m_vertexIndex;
        std::vector<Vertex> m_vertexData;
        Buffer *m_vbo;
        Buffer *m_ibo;
        bool loaded{false};
        bool indexed{false};
        bool buffer_loaded{false};

        friend class Renderer;

    public:
        Geometry() : m_vbo{new Buffer},
                     m_ibo{new Buffer} {}
        inline bool is_data_loaded() const { return loaded; }
        inline bool is_buffer_loaded() const { return buffer_loaded; }
        inline bool is_indexed() const { return indexed; }

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
        void fill(glm::vec3 **pos, glm::vec3 **normal, glm::vec2 **uv, glm::vec3 **tangent);
    };

};

namespace std
{
    template <>
    struct hash<vke::Vertex>
    {
        size_t operator()(vke::Vertex const &vertex) const
        {
            size_t seed = 0;
            vke::vkutils::hash_combine(seed, vertex.pos, vertex.normal, vertex.tangent, vertex.texCoord, vertex.color);
            return seed;
        }
    };
};

#endif // VK_GEOMETRY_H