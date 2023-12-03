#include "vk_buffer.h"

void vke::Buffer::upload_data(VmaAllocator memory, const void *bufferData, size_t size)
{
    void *data;
    vmaMapMemory(memory, allocation, &data);
    memcpy(data, bufferData, size);
    vmaUnmapMemory(memory, allocation);
}

void vke::Buffer::upload_data(VmaAllocator memory, const void *bufferData, size_t size, size_t offset)
{
    char *data;
    vmaMapMemory(memory, allocation, (void **)&data);
    data += offset;
    memcpy(data, bufferData, size);
    vmaUnmapMemory(memory, allocation);
}
