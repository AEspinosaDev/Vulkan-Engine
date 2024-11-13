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
struct Buffer {
    VkBuffer handle = VK_NULL_HANDLE;
    /*Memory allocation controlled by VMA*/
    VmaAllocator          memory     = VK_NULL_HANDLE;
    VmaAllocation         allocation = VK_NULL_HANDLE;
    uint32_t              size       = 0;
    uint32_t              strideSize = 0;
    std::vector<uint32_t> partitionsSize;

    Buffer() {
    }

    void init(VmaAllocator&      _memory,
              size_t             allocSize,
              VkBufferUsageFlags usage,
              VmaMemoryUsage     memoryUsage,
              uint32_t           istrideSize = 0);
    void init(VmaAllocator&         _memory,
              size_t                allocSize,
              VkBufferUsageFlags    usage,
              VmaMemoryUsage        memoryUsage,
              uint32_t              istrideSize,
              std::vector<uint32_t> stridePartitionsSizes);

    void upload_data(const void* bufferData, size_t size);

    void upload_data(const void* bufferData, size_t size, size_t offset);

    void cleanup();

    uint64_t get_device_address(VkDevice device);
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif