#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include "vk_bootstrap.h"

namespace vke
{
    /*Vulkan buffer object*/
    struct Buffer
    {
        VkBuffer buffer;
        /*Memory allocation controlled by VMA*/
        VmaAllocation allocation;
        /*For dynamic descriptor binding operation*/
        uint32_t strideSize{0};
        /*For buffer info writing operation*/
        std::vector<int> stridePartitions;

        void init(VmaAllocator memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,uint32_t istrideSize = 0);
        void init(VmaAllocator memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,uint32_t istrideSize,  uint32_t* istrideDataSizes);
        
        void upload_data(VmaAllocator memory, const void *bufferData, size_t size);

        void upload_data(VmaAllocator memory, const void *bufferData, size_t size, size_t offset);


        void cleanup(VmaAllocator memory);
    };
}

#endif