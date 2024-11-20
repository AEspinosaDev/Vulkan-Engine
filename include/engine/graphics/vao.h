/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VAO_H
#define VAO_H

#include <engine/graphics/buffer.h>
#include <engine/graphics/utilities/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
/*
Geometric Render Data
*/
struct VertexArrays {
    bool loadedOnGPU = false;

    Buffer   vbo         = {};
    uint32_t vertexCount = 0;
    Buffer   ibo         = {};
    uint32_t indexCount  = 0;

    /*
    Optional, if the geometry need a proxy axis-aligned voxelized volume
    */
    Buffer   voxelBuffer   = {};
    uint32_t voxelCount    = 0;
};
typedef VertexArrays VAO;
/*
Canonical vertex definition. Most meshes will use some or all of these attributes.
*/
struct Vertex {
    Vec3 pos;
    Vec3 normal;
    Vec3 tangent;
    Vec2 texCoord;
    Vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding   = 0;
        bindingDescription.stride    = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(bool position = true,
                                                                                   bool normal   = true,
                                                                                   bool tangent  = true,
                                                                                   bool texCoord = true,
                                                                                   bool color    = true) {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        if (position)
        {
            VkVertexInputAttributeDescription posAtt{};
            posAtt.binding  = 0;
            posAtt.location = 0;
            posAtt.format   = VK_FORMAT_R32G32B32_SFLOAT;
            posAtt.offset   = offsetof(Vertex, pos);
            attributeDescriptions.push_back(posAtt);
        }
        if (normal)
        {
            VkVertexInputAttributeDescription normalAtt{};
            normalAtt.binding  = 0;
            normalAtt.location = 1;
            normalAtt.format   = VK_FORMAT_R32G32B32_SFLOAT;
            normalAtt.offset   = offsetof(Vertex, normal);
            attributeDescriptions.push_back(normalAtt);
        }
        if (texCoord)
        {
            VkVertexInputAttributeDescription texCoordAtt{};
            texCoordAtt.binding  = 0;
            texCoordAtt.location = 2;
            texCoordAtt.format   = VK_FORMAT_R32G32_SFLOAT;
            texCoordAtt.offset   = offsetof(Vertex, texCoord);
            attributeDescriptions.push_back(texCoordAtt);
        }
        if (tangent)
        {
            VkVertexInputAttributeDescription tangentAtt{};
            tangentAtt.binding  = 0;
            tangentAtt.location = 3;
            tangentAtt.format   = VK_FORMAT_R32G32B32_SFLOAT;
            tangentAtt.offset   = offsetof(Vertex, tangent);
            attributeDescriptions.push_back(tangentAtt);
        }
        if (color)
        {
            VkVertexInputAttributeDescription colorAtt{};
            colorAtt.binding  = 0;
            colorAtt.location = 4;
            colorAtt.format   = VK_FORMAT_R32G32B32_SFLOAT;
            colorAtt.offset   = offsetof(Vertex, color);
            attributeDescriptions.push_back(colorAtt);
        }

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && normal == other.normal && tangent == other.tangent && texCoord == other.texCoord &&
               color == other.color;
    }

    bool operator!=(const Vertex& other) const {
        return !(*this == other);
    }
};
/*
Voxel data type configured as an Axis-Aligned-Box
*/
struct Voxel {
    Vec3 minCoord = Vec3(0.0f);
    Vec3 maxCoord = Vec3(0.0f);
    Voxel() {
    }
    Voxel(Vec3 center, float radius) {
        maxCoord = Vec3(center + radius);
        minCoord = Vec3(center - radius);
    }
};

template <typename T, typename... Rest> void hash_combine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

namespace std {
template <> struct hash<VKFW::Graphics::Vertex> {
    size_t operator()(VKFW::Graphics::Vertex const& vertex) const {
        size_t seed = 0;
        VKFW::Graphics::hash_combine(seed, vertex.pos, vertex.normal, vertex.tangent, vertex.texCoord, vertex.color);
        return seed;
    }
};
}; // namespace std

#endif