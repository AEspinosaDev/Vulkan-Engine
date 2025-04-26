#include <engine/graphics/command_buffer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

CommandBuffer CommandPool::allocate_command_buffer(uint32_t count, CommandBufferLevel level) {
    CommandBuffer cmd                        = {};
    cmd.device                               = device;
    cmd.pool                                 = handle;
    cmd.queue                                = queue;
    VkCommandBufferAllocateInfo cmdAllocInfo = Init::command_buffer_allocate_info(handle, 1, Translator::get(level));
    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmd.handle));
    return cmd;
}

void CommandPool::reset(VkCommandPoolResetFlags flags) const {
    if (vkResetCommandPool(device, handle, flags) != VK_SUCCESS)
    {
        throw VKFW_Exception("Failed to reset command pool!");
    }
}

void CommandPool::cleanup() {
    if (handle)
    {
        vkDestroyCommandPool(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}
void CommandBuffer::cleanup() {
    if (handle)
    {
        vkFreeCommandBuffers(device, pool, 1, &handle);
        handle = VK_NULL_HANDLE;
    }
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags) {
    if (isRecording)
    {
        throw VKFW_Exception("Command buffer is already recording!");
    }

    VkCommandBufferBeginInfo beginInfo = Init::command_buffer_begin_info();

    if (vkBeginCommandBuffer(handle, &beginInfo) != VK_SUCCESS)
    {
        throw VKFW_Exception("Failed to begin recording command buffer!");
    }
    isRecording = true;
}

void CommandBuffer::end() {
    if (!isRecording)
    {
        throw VKFW_Exception("Command buffer is not recording!");
    }

    if (vkEndCommandBuffer(handle) != VK_SUCCESS)
    {
        throw VKFW_Exception("Failed to end recording command buffer!");
    }
    isRecording = false;
}

void CommandBuffer::reset() {
    if (vkResetCommandBuffer(handle, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to reset command buffer!");
    }
    isRecording = false;
}

void CommandBuffer::submit(Fence                  fence,
                           std::vector<Semaphore> waitSemaphores,
                           std::vector<Semaphore> signalSemaphores) {

    std::vector<VkSemaphore> signalSemaphoreHandles;
    signalSemaphoreHandles.resize(signalSemaphores.size());
    for (size_t i = 0; i < signalSemaphores.size(); i++)
    {
        signalSemaphoreHandles[i] = signalSemaphores[i].handle;
    }
    std::vector<VkSemaphore> waitSemaphoreHandles;
    waitSemaphoreHandles.resize(waitSemaphores.size());
    for (size_t i = 0; i < waitSemaphores.size(); i++)
    {
        waitSemaphoreHandles[i] = waitSemaphores[i].handle;
    }

    VkSubmitInfo submitInfo = Init::submit_info(&handle);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    if (!waitSemaphoreHandles.empty())
    {
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphoreHandles.size());
        submitInfo.pWaitSemaphores    = waitSemaphoreHandles.data();
        submitInfo.pWaitDstStageMask  = waitStages;
    }

    if (!signalSemaphoreHandles.empty())
    {
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphoreHandles.size());
        submitInfo.pSignalSemaphores    = signalSemaphoreHandles.data();
    }

    if (vkQueueSubmit(queue, 1, &submitInfo, fence.handle ? fence.handle : VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw VKFW_Exception("Failed to submit command buffer!");
    }
}

void CommandBuffer::begin_renderpass(RenderPass& renderpass, Framebuffer& fbo, VkSubpassContents subpassContents) {

    VkRenderPassBeginInfo renderPassInfo = Init::renderpass_begin_info(renderpass.handle, fbo.extent, fbo.handle);

    std::vector<VkClearValue> clearValues;
    clearValues.reserve(renderpass.attachmentsConfig.size());

    for (size_t i = 0; i < fbo.attachmentImagesPtrs.size(); i++)
    {
        fbo.attachmentImagesPtrs[i]->currentLayout = renderpass.attachmentsConfig[i].initialLayout;
        clearValues.push_back(fbo.attachmentImagesPtrs[i]->clearValue);
        // clearValues.push_back( renderpass.attachmentsConfig[i].imageConfig.clearValue);
    }

    renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(handle, &renderPassInfo, subpassContents);
}
void CommandBuffer::end_renderpass(RenderPass& renderpass, Framebuffer& fbo) {
    for (size_t i = 0; i < fbo.attachmentImagesPtrs.size(); i++)
    {
        fbo.attachmentImagesPtrs[i]->currentLayout = renderpass.attachmentsConfig[i].finalLayout;
    }
    vkCmdEndRenderPass(handle);
}
void CommandBuffer::draw_geometry(VertexArrays& vao,
                                  uint32_t      instanceCount,
                                  uint32_t      firstOcurrence,
                                  int32_t       offset,
                                  uint32_t      firstInstance) {
    if (!vao.loadedOnGPU)
        return;
    PROFILING_EVENT()

    VkBuffer     vertexBuffers[] = {vao.vbo.handle};
    VkDeviceSize offsets[]       = {0};
    vkCmdBindVertexBuffers(handle, 0, 1, vertexBuffers, offsets);

    if (vao.indexCount > 0)
    {
        vkCmdBindIndexBuffer(handle, vao.ibo.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(handle, vao.indexCount, instanceCount, firstOcurrence, offset, firstInstance);
    } else
    {
        vkCmdDraw(handle, vao.vertexCount, instanceCount, firstOcurrence, firstInstance);
    }
}
void CommandBuffer::draw_gui_data() {
    if (ImGui::GetDrawData())
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), handle);
}
void CommandBuffer::bind_shaderpass(ShaderPass& pass) {
    switch (pass.QUEUE_TYPE)
    {
    case GRAPHIC_QUEUE:
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.pipeline);
        break;
    case COMPUTE_QUEUE:
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pass.pipeline);
        break;
    case RT_QUEUE:
        vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pass.pipeline);
        break;
    default:
        break;
    }
}
void CommandBuffer::bind_descriptor_set(DescriptorSet         descriptor,
                                        uint32_t              ocurrence,
                                        ShaderPass&           pass,
                                        std::vector<uint32_t> offsets,
                                        BindingType           binding) {
    vkCmdBindDescriptorSets(handle,
                            static_cast<VkPipelineBindPoint>(binding),
                            pass.pipelineLayout,
                            ocurrence,
                            1,
                            &descriptor.handle,
                            offsets.size(),
                            offsets.data());
}
void CommandBuffer::set_viewport(Extent2D extent, Offset2D scissorOffset) {
    VkViewport viewport = Init::viewport(extent);
    vkCmdSetViewport(handle, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = scissorOffset;
    scissor.extent = extent;
    vkCmdSetScissor(handle, 0, 1, &scissor);
}
void CommandBuffer::set_cull_mode(CullingMode mode) {
    vkCmdSetCullMode(handle, (VkCullModeFlags)mode);
}

void CommandBuffer::set_depth_write_enable(bool op) {
    vkCmdSetDepthWriteEnable(handle, op);
}

void CommandBuffer::set_depth_test_enable(bool op) {
    vkCmdSetDepthTestEnable(handle, op);
}
void CommandBuffer::set_depth_bias_enable(bool op) {
    vkCmdSetDepthBiasEnable(handle, true);
}
void CommandBuffer::set_depth_bias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    vkCmdSetDepthBias(handle, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
} // namespace Graphics

void Graphics::CommandBuffer::pipeline_barrier(Image&        img,
                                               ImageLayout   oldLayout,
                                               ImageLayout   newLayout,
                                               AccessFlags   srcMask,
                                               AccessFlags   dstMask,
                                               PipelineStage srcStage,
                                               PipelineStage dstStage) {

    VkImageMemoryBarrier barrier            = {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = Translator::get(oldLayout);
    barrier.newLayout                       = Translator::get(newLayout);
    barrier.srcAccessMask                   = Translator::get(srcMask);
    barrier.dstAccessMask                   = Translator::get(dstMask);
    barrier.image                           = img.handle;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = img.baseMipLevel;
    barrier.subresourceRange.levelCount     = img.mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = img.layers;

    vkCmdPipelineBarrier(
        handle, Translator::get(srcStage), Translator::get(dstStage), 0, 0, nullptr, 0, nullptr, 1, &barrier);

    img.currentLayout = newLayout;
}
void Graphics::CommandBuffer::pipeline_barrier(Image&        img,
                                               uint32_t      baseMipLevel,
                                               uint32_t      mipLevels,
                                               ImageLayout   oldLayout,
                                               ImageLayout   newLayout,
                                               AccessFlags   srcMask,
                                               AccessFlags   dstMask,
                                               PipelineStage srcStage,
                                               PipelineStage dstStage) {

    VkImageMemoryBarrier barrier            = {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = Translator::get(oldLayout);
    barrier.newLayout                       = Translator::get(newLayout);
    barrier.srcAccessMask                   = Translator::get(srcMask);
    barrier.dstAccessMask                   = Translator::get(dstMask);
    barrier.image                           = img.handle;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = baseMipLevel;
    barrier.subresourceRange.levelCount     = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = img.layers;

    vkCmdPipelineBarrier(
        handle, Translator::get(srcStage), Translator::get(dstStage), 0, 0, nullptr, 0, nullptr, 1, &barrier);

    img.currentLayout = newLayout;
}
void Graphics::CommandBuffer::clear_image(Image& img, ImageLayout layout, ImageAspect aspect, Vec4 clearColor) {

    VkClearColorValue vclearColor = {};
    vclearColor.float32[0]        = clearColor.r;
    vclearColor.float32[1]        = clearColor.g;
    vclearColor.float32[2]        = clearColor.b;
    vclearColor.float32[3]        = clearColor.a;

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask              = Translator::get(aspect);
    subresourceRange.baseMipLevel            = img.baseMipLevel;
    subresourceRange.levelCount              = img.mipLevels;
    subresourceRange.baseArrayLayer          = 0;
    subresourceRange.layerCount              = img.layers;

    vkCmdClearColorImage(handle, img.handle, Translator::get(layout), &vclearColor, 1, &subresourceRange);
}
void Graphics::CommandBuffer::blit_image(Image&      srcImage,
                                         Image&      dstImage,
                                         FilterType  filter,
                                         uint32_t    mipLevel,
                                         ImageAspect srcAspect,
                                         ImageAspect dstAspect) {
    VkImageBlit blitRegion   = {};
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {
        static_cast<int32_t>(srcImage.extent.width), static_cast<int32_t>(srcImage.extent.height), 1};
    blitRegion.srcSubresource.aspectMask     = Translator::get(srcAspect);
    blitRegion.srcSubresource.mipLevel       = mipLevel;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount     = 1;

    blitRegion.dstOffsets[0] = {0, 0, 0};
    blitRegion.dstOffsets[1] = {
        static_cast<int32_t>(dstImage.extent.width), static_cast<int32_t>(dstImage.extent.height), 1};
    blitRegion.dstSubresource.aspectMask     = Translator::get(dstAspect);
    blitRegion.dstSubresource.mipLevel       = mipLevel;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount     = 1;

    vkCmdBlitImage(handle,
                   srcImage.handle,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstImage.handle,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1,
                   &blitRegion,
                   Translator::get(filter));
}
void Graphics::CommandBuffer::blit_image(Image&      srcImage,
                                         Image&      dstImage,
                                         Extent2D    srcOrigin,
                                         Extent2D    dstOrigin,
                                         Extent2D    srcExtent,
                                         Extent2D    dstExtent,
                                         FilterType  filter,
                                         uint32_t    mipLevel,
                                         ImageAspect srcAspect,
                                         ImageAspect dstAspect) {

    VkImageBlit blitRegion   = {};
    blitRegion.srcOffsets[0] = {static_cast<int32_t>(srcOrigin.width), static_cast<int32_t>(srcOrigin.height), 0};
    blitRegion.srcOffsets[1] = {static_cast<int32_t>(srcExtent.width), static_cast<int32_t>(srcExtent.height), 1};
    blitRegion.srcSubresource.aspectMask     = Translator::get(srcAspect);
    blitRegion.srcSubresource.mipLevel       = mipLevel;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount     = 1;

    blitRegion.dstOffsets[0] = {static_cast<int32_t>(dstOrigin.width), static_cast<int32_t>(dstOrigin.height), 0};
    blitRegion.dstOffsets[1] = {static_cast<int32_t>(dstExtent.width), static_cast<int32_t>(dstExtent.height), 1};
    blitRegion.dstSubresource.aspectMask     = Translator::get(dstAspect);
    blitRegion.dstSubresource.mipLevel       = mipLevel;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount     = 1;

    vkCmdBlitImage(handle,
                   srcImage.handle,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstImage.handle,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1,
                   &blitRegion,
                   Translator::get(filter));
}
void Graphics::CommandBuffer::push_constants(ShaderPass&      pass,
                                             ShaderStageFlags stage,
                                             const void*      data,
                                             uint32_t         size,
                                             uint32_t         offset) {
    vkCmdPushConstants(handle, pass.pipelineLayout, Translator::get(stage), offset, size, data);
}

void Graphics::CommandBuffer::dispatch_compute(Extent3D grid) {
    vkCmdDispatch(handle, grid.width, grid.height, grid.depth);
}

void Graphics::CommandBuffer::copy_buffer(Buffer& srcBuffer, Buffer& dstBuffer, size_t size) {
    VkBufferCopy copy;
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size      = size;
    vkCmdCopyBuffer(handle, srcBuffer.handle, dstBuffer.handle, 1, &copy);
}

void Graphics::CommandBuffer::copy_buffer_to_image(Image& img, Buffer& buffer) {

    VkImageSubresourceRange range;
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel   = 0;
    range.levelCount     = img.mipLevels;
    range.baseArrayLayer = 0;
    range.layerCount     = img.layers;

    VkImageMemoryBarrier imageBarrier_toTransfer = {};
    imageBarrier_toTransfer.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    imageBarrier_toTransfer.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrier_toTransfer.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toTransfer.image            = img.handle;
    imageBarrier_toTransfer.subresourceRange = range;

    imageBarrier_toTransfer.srcAccessMask = 0;
    imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // barrier the image into the transfer-receive layout
    vkCmdPipelineBarrier(handle,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &imageBarrier_toTransfer);

    // For each layer
    for (uint32_t layer = 0; layer < img.layers; ++layer)
    {
        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset =
            layer * ((img.extent.width * img.extent.height * buffer.size) / img.layers); // Offset per face
        copyRegion.bufferRowLength   = 0;
        copyRegion.bufferImageHeight = 0;

        copyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel       = 0;
        copyRegion.imageSubresource.baseArrayLayer = layer;
        copyRegion.imageSubresource.layerCount     = 1;
        copyRegion.imageExtent                     = img.extent;

        vkCmdCopyBufferToImage(handle, buffer.handle, img.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    }

    if (img.mipLevels == 1)
    {
        VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

        imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // barrier the image into the shader readable layout
        vkCmdPipelineBarrier(handle,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &imageBarrier_toReadable);
    }
}

void Graphics::CommandBuffer::generate_mipmaps(Image& img, ImageLayout initialLayout, ImageLayout finalLayout) {

    int32_t mipWidth  = img.extent.width;
    int32_t mipHeight = img.extent.height;
    int32_t mipDepth  = img.extent.depth;

    VkImageSubresourceRange range;
    range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    range.levelCount     = 1;
    range.baseArrayLayer = 0;
    range.layerCount     = img.layers;

    VkImageMemoryBarrier imageBarrier_toTransfer = {};
    imageBarrier_toTransfer.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier_toTransfer.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier_toTransfer.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier_toTransfer.image                = img.handle;
    imageBarrier_toTransfer.subresourceRange     = range;

    for (uint32_t i = 1; i < img.mipLevels; i++)
    {
        for (uint32_t layer = 0; layer < img.layers; ++layer)
        {
            // Set barriers for the current face
            imageBarrier_toTransfer.subresourceRange.baseMipLevel   = i - 1;
            imageBarrier_toTransfer.subresourceRange.baseArrayLayer = layer;
            imageBarrier_toTransfer.oldLayout                       = Translator::get(initialLayout);
            imageBarrier_toTransfer.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imageBarrier_toTransfer.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toTransfer.dstAccessMask                   = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(handle,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &imageBarrier_toTransfer);

            VkImageBlit blit{};
            blit.srcOffsets[0]                 = {0, 0, 0};
            blit.srcOffsets[1]                 = {mipWidth, mipHeight, mipDepth};
            blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel       = i - 1;
            blit.srcSubresource.baseArrayLayer = layer; // Specify the current face
            blit.srcSubresource.layerCount     = 1;

            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {
                mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1};
            blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel       = i;
            blit.dstSubresource.baseArrayLayer = layer; // Specify the current face
            blit.dstSubresource.layerCount     = 1;

            vkCmdBlitImage(handle,
                           img.handle,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           img.handle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &blit,
                           VK_FILTER_LINEAR);
        }

        imageBarrier_toTransfer.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageBarrier_toTransfer.newLayout     = Translator::get(finalLayout);
        imageBarrier_toTransfer.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(handle,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &imageBarrier_toTransfer);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
        if (mipDepth > 1)
            mipDepth /= 2;
    }

    VkImageMemoryBarrier imageBarrier_toReadable          = imageBarrier_toTransfer;
    imageBarrier_toReadable.subresourceRange.baseMipLevel = img.mipLevels - 1;
    imageBarrier_toReadable.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageBarrier_toReadable.newLayout                     = Translator::get(finalLayout);
    imageBarrier_toReadable.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
    imageBarrier_toReadable.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(handle,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &imageBarrier_toReadable);

    img.currentLayout = finalLayout;
}
VULKAN_ENGINE_NAMESPACE_END