#include <engine/graphics/command_buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{
void CommandPool::init(VkDevice &device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
    m_device = device;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = flags;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        throw VKException("Failed to create command pool!");
    }
}
CommandBuffer CommandPool::allocate_command_buffer(uint32_t count, VkCommandBufferLevel level)
{
    CommandBuffer commandBuffer;
    commandBuffer.init(m_device, m_handle, level);
    return commandBuffer;
}

void CommandPool::reset(VkCommandPoolResetFlags flags) const
{
    if (vkResetCommandPool(m_device, m_handle, flags) != VK_SUCCESS)
    {
        throw VKException("Failed to reset command pool!");
    }
}

CommandBuffer::~CommandBuffer()
{
    if (m_handle != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(m_device, m_pool, 1, &m_handle);
    }
}

void CommandBuffer::init(VkDevice &device, VkCommandPool &commandPool, VkCommandBufferLevel level)
{
    m_device = device;
    m_pool = commandPool;

    VkCommandBufferAllocateInfo cmdAllocInfo = Init::command_buffer_allocate_info(commandPool, 1, level);

    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &m_handle));
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags)
{
    if (m_isRecording)
    {
        throw VKException("Command buffer is already recording!");
    }

    VkCommandBufferBeginInfo beginInfo = Init::command_buffer_begin_info();

    if (vkBeginCommandBuffer(m_handle, &beginInfo) != VK_SUCCESS)
    {
        throw VKException("Failed to begin recording command buffer!");
    }
    m_isRecording = true;
}

void CommandBuffer::end()
{
    if (!m_isRecording)
    {
        throw VKException("Command buffer is not recording!");
    }

    if (vkEndCommandBuffer(m_handle) != VK_SUCCESS)
    {
        throw VKException("Failed to end recording command buffer!");
    }
    m_isRecording = false;
}

void CommandBuffer::reset()
{
    if (vkResetCommandBuffer(m_handle, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset command buffer!");
    }
    m_isRecording = false;
}

void CommandBuffer::submit(VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence)
{
    VkSubmitInfo submitInfo = Init::submit_info(&m_handle);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    if (waitSemaphore != VK_NULL_HANDLE)
    {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
    }

    if (signalSemaphore != VK_NULL_HANDLE)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;
    }

    if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw VKException("Failed to submit command buffer!");
    }
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END