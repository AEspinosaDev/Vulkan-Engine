/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMMAND_BUFFER_H
#define COMMAND_BUFFER_H

#include <engine/common.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

class CommandBuffer
{
    VkCommandBuffer m_handle{VK_NULL_HANDLE};

    VkDevice m_device;
    VkCommandPool m_pool;
    bool m_isRecording{false};

  public:
    CommandBuffer(){}

    ~CommandBuffer();

    void init(VkDevice &device, VkCommandPool &commandPool,
                  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    void end();
    void reset();

    void submit(VkQueue queue, VkSemaphore waitSemaphore = VK_NULL_HANDLE, VkSemaphore signalSemaphore = VK_NULL_HANDLE,
                VkFence fence = VK_NULL_HANDLE);

    VkCommandBuffer get_handle() const
    {
        return m_handle;
    }
};


} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif