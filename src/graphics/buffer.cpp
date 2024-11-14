#include <engine/graphics/buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Buffer::upload_data(const void* bufferData, size_t size) {
    PROFILING_EVENT()
    if (!bufferData)
        return;
    if (allocation)
    {
        void* data;
        VK_CHECK(vmaMapMemory(allocator, allocation, &data));
        memcpy(data, bufferData, size);
        vmaUnmapMemory(allocator, allocation);
    }
    if (memory)
    {
        void* data;
        VK_CHECK(vkMapMemory(device, memory, 0, size, 0, &data));
        memcpy(data, bufferData, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if (!coherence)
        {
            VkMappedMemoryRange mappedRange{};
            mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = memory;
            mappedRange.offset = 0;
            mappedRange.size   = size;
            vkFlushMappedMemoryRanges(device, 1, &mappedRange);
        }
        vkUnmapMemory(device, memory);
    }
}

void Buffer::upload_data(const void* bufferData, size_t size, size_t offset) {
    PROFILING_EVENT()
    if (!bufferData)
        return;
    if (allocation)
    {
        char* data;
        VK_CHECK(vmaMapMemory(allocator, allocation, (void**)&data));
        data += offset;
        memcpy(data, bufferData, size);
        vmaUnmapMemory(allocator, allocation);
    }
    if (memory)
    {
        void* data;
        VK_CHECK(vkMapMemory(device, memory, offset, size, 0, &data));
        memcpy(data, bufferData, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if (!coherence)
        {
            VkMappedMemoryRange mappedRange{};
            mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mappedRange.memory = memory;
            mappedRange.offset = offset;
            mappedRange.size   = size;
            vkFlushMappedMemoryRanges(device, 1, &mappedRange);
        }
        vkUnmapMemory(device, memory);
    }
}

uint64_t Buffer::get_device_address() {
    VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
    bufferDeviceAI.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = handle;
    return vkGetBufferDeviceAddress(device, &bufferDeviceAI);
}
void Buffer::cleanup() {
    if (allocation)
    {
        vmaDestroyBuffer(allocator, handle, allocation);
        allocation = VK_NULL_HANDLE;
    }
    if (memory)
    {
        if (handle)
        {
            vkDestroyBuffer(device, handle, nullptr);
            handle = VK_NULL_HANDLE;
        }
        vkFreeMemory(device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END