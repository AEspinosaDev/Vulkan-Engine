#include <engine/graphics/buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Buffer::upload_data(VmaAllocator &memory, const void *bufferData, size_t size)
{
    void *data;
    vmaMapMemory(memory, allocation, &data);
    memcpy(data, bufferData, size);
    vmaUnmapMemory(memory, allocation);
}

void Buffer::upload_data(VmaAllocator &memory, const void *bufferData, size_t size, size_t offset)
{
    char *data;
    vmaMapMemory(memory, allocation, (void **)&data);
    data += offset;
    memcpy(data, bufferData, size);
    vmaUnmapMemory(memory, allocation);
}

void Buffer::init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t istrideSize)
{

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo,
                             &buffer,
                             &allocation,
                             nullptr));

    strideSize = istrideSize;
}

void Buffer::init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t istrideSize, std::vector<uint32_t> stridePartitionsSizes)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo,
                             &buffer,
                             &allocation,
                             nullptr));

    strideSize = istrideSize;
    partitionsSizes = stridePartitionsSizes;
}

void Buffer::cleanup(VmaAllocator &memory)
{
    vmaDestroyBuffer(memory, buffer,
                     allocation);
}

VULKAN_ENGINE_NAMESPACE_END