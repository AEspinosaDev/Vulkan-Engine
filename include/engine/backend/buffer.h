/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BUFFER_H
#define BUFFER_H

#include <engine/backend/bootstrap.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
/*Vulkan buffer object*/
struct Buffer
{
    VkBuffer buffer;
    /*Memory allocation controlled by VMA*/
    VmaAllocation allocation;
    /*For dynamic descriptor binding operation*/
    uint32_t strideSize{0};
    /*For buffer info writing operation*/
    std::vector<uint32_t> partitionsSizes;

    void init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t istrideSize = 0);
    void init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t istrideSize, std::vector<uint32_t> stridePartitionsSizes);

    void upload_data(VmaAllocator &memory, const void *bufferData, size_t size);

    void upload_data(VmaAllocator &memory, const void *bufferData, size_t size, size_t offset);

    void cleanup(VmaAllocator &memory);
};

VULKAN_ENGINE_NAMESPACE_END

#endif