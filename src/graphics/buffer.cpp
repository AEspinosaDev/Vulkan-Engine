#include <engine/graphics/buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Buffer::upload_data(const void* bufferData, size_t size) {
    if (allocation == VK_NULL_HANDLE)
        return;
    PROFILING_EVENT()
    void* data;
    vmaMapMemory(memory, allocation, &data);
    memcpy(data, bufferData, size);
    vmaUnmapMemory(memory, allocation);
}

void Buffer::upload_data(const void* bufferData, size_t size, size_t offset) {
    if (allocation == VK_NULL_HANDLE)
        return;
    PROFILING_EVENT()
    char* data;
    vmaMapMemory(memory, allocation, (void**)&data);
    data += offset;
    memcpy(data, bufferData, size);
    vmaUnmapMemory(memory, allocation);
}

void Buffer::init(VmaAllocator&      _memory,
                  size_t             allocSize,
                  VkBufferUsageFlags usage,
                  VmaMemoryUsage     memoryUsage,
                  uint32_t           istrideSize) {
    memory = _memory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext              = nullptr;

    bufferInfo.size  = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage                   = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo, &handle, &allocation, nullptr));

    strideSize = istrideSize;
    size = allocSize;
}

void Buffer::init(VmaAllocator&         _memory,
                  size_t                allocSize,
                  VkBufferUsageFlags    usage,
                  VmaMemoryUsage        memoryUsage,
                  uint32_t              istrideSize,
                  std::vector<uint32_t> stridePartitionsSizes) {
    memory = _memory;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext              = nullptr;

    bufferInfo.size  = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage                   = memoryUsage;

    VK_CHECK(vmaCreateBuffer(memory, &bufferInfo, &vmaallocInfo, &handle, &allocation, nullptr));

    size = allocSize;
    strideSize      = istrideSize;
    partitionsSize = stridePartitionsSizes;
}

void Buffer::cleanup() {
    if (allocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(memory, handle, allocation);
        allocation = VK_NULL_HANDLE;
    }
}

uint64_t Buffer::get_device_address(VkDevice device) {
    VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
    bufferDeviceAI.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = handle;
    return vkGetBufferDeviceAddress(device, &bufferDeviceAI);
    
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END