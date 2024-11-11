#include <engine/graphics/command_buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
void CommandPool::init(VkDevice& device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
    m_device = device;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags            = flags;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_handle) != VK_SUCCESS)
    {
        throw VKException("Failed to create command pool!");
    }
}
CommandBuffer CommandPool::allocate_command_buffer(uint32_t count, VkCommandBufferLevel level) {
    CommandBuffer commandBuffer;
    commandBuffer.init(m_device, *this, level);
    return commandBuffer;
}

void CommandPool::reset(VkCommandPoolResetFlags flags) const {
    if (vkResetCommandPool(m_device, m_handle, flags) != VK_SUCCESS)
    {
        throw VKException("Failed to reset command pool!");
    }
}

void CommandPool::cleanup() {
    if (m_handle != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }
}
CommandBuffer::~CommandBuffer() {
    if (m_handle != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(m_device, m_pool, 1, &m_handle);
        m_handle = VK_NULL_HANDLE;
    }
}

void CommandBuffer::init(VkDevice device, CommandPool commandPool, VkCommandBufferLevel level) {
    m_device = device;
    m_pool   = commandPool.get_handle();

    VkCommandBufferAllocateInfo cmdAllocInfo = Init::command_buffer_allocate_info(m_pool, 1, level);

    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &m_handle));
}

void CommandBuffer::begin(VkFence& renderFence, VkCommandBufferUsageFlags flags) {
    if (m_isRecording)
    {
        throw VKException("Command buffer is already recording!");
    }

    VK_CHECK(vkResetFences(m_device, 1, &renderFence));
    VK_CHECK(vkResetCommandBuffer(m_handle, 0));

    VkCommandBufferBeginInfo beginInfo = Init::command_buffer_begin_info();

    if (vkBeginCommandBuffer(m_handle, &beginInfo) != VK_SUCCESS)
    {
        throw VKException("Failed to begin recording command buffer!");
    }
    m_isRecording = true;
}

void CommandBuffer::end() {
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

void CommandBuffer::reset() {
    if (vkResetCommandBuffer(m_handle, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset command buffer!");
    }
    m_isRecording = false;
}

void CommandBuffer::submit(VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence) {
    VkSubmitInfo submitInfo = Init::submit_info(&m_handle);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    if (waitSemaphore != VK_NULL_HANDLE)
    {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores    = &waitSemaphore;
        submitInfo.pWaitDstStageMask  = waitStages;
    }

    if (signalSemaphore != VK_NULL_HANDLE)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = &signalSemaphore;
    }

    if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw VKException("Failed to submit command buffer!");
    }
}

void CommandBuffer::begin_renderpass(VulkanRenderPass&        renderpass,
                                     Framebuffer&             fbo,
                                     std::vector<Attachment>& attachments,
                                     VkSubpassContents        subpassContents) {

    VkRenderPassBeginInfo renderPassInfo =
        Init::renderpass_begin_info(renderpass.get_handle(),
                                    {attachments.front().image.extent.width, attachments.front().image.extent.height},
                                    fbo.get_handle());

    std::vector<VkClearValue> clearValues;
    clearValues.reserve(attachments.size());
    for (size_t i = 0; i < attachments.size(); i++)
    {
        clearValues.push_back(attachments[i].clearValue);
    }

    renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(m_handle, &renderPassInfo, subpassContents);
}
void CommandBuffer::end_renderpass() {

    vkCmdEndRenderPass(m_handle);
}
void CommandBuffer::draw_geometry(VertexArrays& vao,
                                  uint32_t      instanceCount,
                                  uint32_t      firstOcurrence,
                                  int32_t       offset,
                                  uint32_t      firstInstance) {
    if (!vao.loadedOnGPU)
        return;
    PROFILING_EVENT()

    VkBuffer     vertexBuffers[] = {vao.vbo.get_handle()};
    VkDeviceSize offsets[]       = {0};
    vkCmdBindVertexBuffers(m_handle, 0, 1, vertexBuffers, offsets);

    if (vao.indexCount > 0)
    {
        vkCmdBindIndexBuffer(m_handle, vao.ibo.get_handle(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(m_handle, vao.indexCount, instanceCount, firstOcurrence, offset, firstInstance);
    } else
    {
        vkCmdDraw(m_handle, vao.vertexCount, instanceCount, firstOcurrence, firstInstance);
    }
}
void CommandBuffer::draw_gui_data() {
    if (ImGui::GetDrawData())
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_handle);
}
void CommandBuffer::bind_shaderpass(ShaderPass& pass, BindingType binding) {
    vkCmdBindPipeline(m_handle, static_cast<VkPipelineBindPoint>(binding), pass.get_pipeline());
}
void CommandBuffer::bind_descriptor_set(DescriptorSet         descriptor,
                                        uint32_t              ocurrence,
                                        ShaderPass&           pass,
                                        std::vector<uint32_t> offsets,
                                        BindingType           binding) {
    vkCmdBindDescriptorSets(m_handle,
                            static_cast<VkPipelineBindPoint>(binding),
                            pass.get_layout(),
                            ocurrence,
                            1,
                            &descriptor.handle,
                            offsets.size(),
                            offsets.data());
}
void CommandBuffer::set_viewport(Extent2D extent, Offset2D scissorOffset) {
    VkViewport viewport = Init::viewport(extent);
    vkCmdSetViewport(m_handle, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = scissorOffset;
    scissor.extent = extent;
    vkCmdSetScissor(m_handle, 0, 1, &scissor);
}
void CommandBuffer::set_cull_mode(CullingMode mode) {
    vkCmdSetCullMode(m_handle, (VkCullModeFlags)mode);
}

void CommandBuffer::set_depth_write_enable(bool op) {
    vkCmdSetDepthWriteEnable(m_handle, op);
}

void CommandBuffer::set_depth_test_enable(bool op) {
    vkCmdSetDepthTestEnable(m_handle, op);
}
void CommandBuffer::set_depth_bias_enable(bool op) {
    vkCmdSetDepthBiasEnable(m_handle, true);
}
void CommandBuffer::set_depth_bias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    vkCmdSetDepthBias(m_handle, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END