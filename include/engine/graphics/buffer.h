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
// Vulkan buffer object. Device class should populate the struct
struct Buffer {
    VkBuffer              handle     = VK_NULL_HANDLE;
    uint32_t              size       = 0;
    uint32_t              strideSize = 0;
    std::vector<uint32_t> partitionsSize;

    /*IF Memory allocation is controlled by VMA*/
    VmaAllocator  allocator  = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    /*IF using Vulkan API*/
    VkDevice       device    = VK_NULL_HANDLE;
    VkDeviceMemory memory    = VK_NULL_HANDLE;
    bool           coherence = false;

    void     upload_data( const void* bufferData, size_t size ) const;
    void     upload_data( const void* bufferData, size_t size, size_t offset ) const;
    void     copy_to( void* data ) const;
    uint64_t get_device_address() const;
    void     cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif