/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BUFFER_H
#define BUFFER_H

#include <engine/graphics/utilities/bootstrap.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
// Vulkan buffer object
class Buffer
{
    VkBuffer m_handle;
    /*Memory allocation controlled by VMA*/
    VmaAllocator  m_memory;
    VmaAllocation m_allocation;

    uint32_t              m_strideSize{0};
    std::vector<uint32_t> m_partitionsSizes;

  public:
    inline VkBuffer get_handle() const {
        return m_handle;
    }
    inline uint32_t get_stride_size() const {
        return m_strideSize;
    }
    Buffer() {}

    void init(VmaAllocator& memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
              uint32_t istrideSize = 0);
    void init(VmaAllocator& memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
              uint32_t istrideSize, std::vector<uint32_t> stridePartitionsSizes);

    void upload_data(const void* bufferData, size_t size);

    void upload_data(const void* bufferData, size_t size, size_t offset);

    void cleanup();
};

/*
Geometric Render Data
*/
struct VertexArrays {
    bool loadedOnGPU{false};

    Buffer vbo{};
    Buffer ibo{};

    uint32_t vertexCount{0};
    uint32_t indexCount{0};
};
typedef VertexArrays VAO;

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif