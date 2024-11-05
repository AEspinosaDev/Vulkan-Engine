#include <engine/graphics/buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

void Buffer::upload_data(const void *bufferData, size_t size)
{
    if(allocation == VK_NULL_HANDLE) return;
    PROFILING_EVENT()
    void *data;
    vmaMapMemory(_memory, allocation, &data);
    memcpy(data, bufferData, size);
    vmaUnmapMemory(_memory, allocation);
}

void Buffer::upload_data(const void *bufferData, size_t size, size_t offset)
{
    if(allocation == VK_NULL_HANDLE) return;
    PROFILING_EVENT()
    char *data;
    vmaMapMemory(_memory, allocation, (void **)&data);
    data += offset;
    memcpy(data, bufferData, size);
    vmaUnmapMemory(_memory, allocation);
}

void Buffer::init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                  uint32_t istrideSize)
{
    _memory = memory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo, &handle, &allocation, nullptr));

    strideSize = istrideSize;
}

void Buffer::init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                  uint32_t istrideSize, std::vector<uint32_t> stridePartitionsSizes)
{
    _memory = memory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo, &handle, &allocation, nullptr));

    strideSize = istrideSize;
    partitionsSizes = stridePartitionsSizes;
}

void Buffer::cleanup()
{
    if(allocation != VK_NULL_HANDLE)
    vmaDestroyBuffer(_memory, handle, allocation);
}

} // namespace render

VULKAN_ENGINE_NAMESPACE_END