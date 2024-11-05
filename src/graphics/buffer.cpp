#include <engine/graphics/buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

void Buffer::upload_data(const void *bufferData, size_t size)
{
    if(m_allocation == VK_NULL_HANDLE) return;
    PROFILING_EVENT()
    void *data;
    vmaMapMemory(m_memory, m_allocation, &data);
    memcpy(data, bufferData, size);
    vmaUnmapMemory(m_memory, m_allocation);
}

void Buffer::upload_data(const void *bufferData, size_t size, size_t offset)
{
    if(m_allocation == VK_NULL_HANDLE) return;
    PROFILING_EVENT()
    char *data;
    vmaMapMemory(m_memory, m_allocation, (void **)&data);
    data += offset;
    memcpy(data, bufferData, size);
    vmaUnmapMemory(m_memory, m_allocation);
}

void Buffer::init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                  uint32_t istrideSize)
{
    m_memory = memory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo, &m_handle, &m_allocation, nullptr));

    m_strideSize = istrideSize;
}

void Buffer::init(VmaAllocator &memory, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                  uint32_t istrideSize, std::vector<uint32_t> stridePartitionsSizes)
{
    m_memory = memory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo, &m_handle, &m_allocation, nullptr));

    m_strideSize = istrideSize;
    m_partitionsSizes = stridePartitionsSizes;
}

void Buffer::cleanup()
{
    if(m_allocation != VK_NULL_HANDLE)
    vmaDestroyBuffer(m_memory, m_handle, m_allocation);
}

} // namespace render

VULKAN_ENGINE_NAMESPACE_END