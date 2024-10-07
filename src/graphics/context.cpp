#include <engine/graphics/context.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Context::init(GLFWwindow *windowHandle, VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode)
{

    // BOOT Vulkan ------>>>

    boot::VKBooter booter(enableValidationLayers);

    instance = booter.boot_vulkan();
    debugMessenger = booter.create_debug_messenger(instance);

    VK_CHECK(glfwCreateWindowSurface(instance, windowHandle, nullptr, &surface));

    // Get gpu
    gpu = booter.pick_graphics_card_device(instance, surface);

    // Create logical device
    device = booter.create_logical_device(
        graphicsQueue,
        presentQueue,
        gpu,
        utils::get_gpu_features(gpu),
        surface);

    // Setup VMA
    memory = booter.setup_memory(instance, device, gpu);

    uploadContext.init(device, gpu, surface);

    // Create swapchain
    swapchain.create(gpu, device, surface, windowHandle, surfaceExtent, framesPerFlight, presentFormat, presentMode);

    // Init frames with control objects
    frames.resize(framesPerFlight);
    for (size_t i = 0; i < frames.size(); i++)
        frames[i].init(device, gpu, surface);
        
    //------<<<
}

void Context::recreate_swapchain(GLFWwindow *windowHandle, VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode)
{
    swapchain.cleanup(device, memory);
    swapchain.create(gpu, device, surface, windowHandle, surfaceExtent, framesPerFlight, presentFormat, presentMode);
}

void Context::cleanup()
{
    for (size_t i = 0; i < frames.size(); i++)
    {
        frames[i].cleanup(device);
    }

    uploadContext.cleanup(device);

    swapchain.cleanup(device, memory);

    vmaDestroyAllocator(memory);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers)
    {
        utils::destroy_debug_utils_messenger_EXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

VkResult Context::aquire_present_image(const uint32_t &currentFrame, uint32_t &imageIndex)
{
    VK_CHECK(vkWaitForFences(device, 1, &frames[currentFrame].renderFence, VK_TRUE, UINT64_MAX));
    VkResult result = vkAcquireNextImageKHR(device, swapchain.get_handle(), UINT64_MAX, frames[currentFrame].presentSemaphore, VK_NULL_HANDLE, &imageIndex);
    return result;
}

void Context::begin_command_buffer(const uint32_t &currentFrame)
{
    VK_CHECK(vkResetFences(device, 1, &frames[currentFrame].renderFence));
    VK_CHECK(vkResetCommandBuffer(frames[currentFrame].commandBuffer, 0));

    VkCommandBufferBeginInfo beginInfo = init::command_buffer_begin_info();

    if (vkBeginCommandBuffer(frames[currentFrame].commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw VKException("failed to begin recording command buffer!");
    }
}

void Context::end_command_buffer(const uint32_t &currentFrame)
{
    if (vkEndCommandBuffer(frames[currentFrame].commandBuffer) != VK_SUCCESS)
    {
        throw VKException("failed to record command buffer!");
    }
}

VkResult Context::present_image(const uint32_t &currentFrame, uint32_t imageIndex)
{
    VkSubmitInfo submitInfo = init::submit_info(&frames[currentFrame].commandBuffer);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {frames[currentFrame].presentSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    VkSemaphore signalSemaphores[] = {frames[currentFrame].renderSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, frames[currentFrame].renderFence) != VK_SUCCESS)
    {
        throw VKException("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = init::present_info();
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchain.get_handle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

void Context::draw_geometry(VkCommandBuffer &cmd, Buffer &vbo, Buffer &ibo, uint32_t vertexCount, uint32_t indexCount, bool indexed, uint32_t instanceCount, uint32_t firstOcurrence, int32_t offset, uint32_t firstInstance)
{
    VkBuffer vertexBuffers[] = {vbo.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    if (indexed)
    {
        vkCmdBindIndexBuffer(cmd, ibo.buffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstOcurrence, offset, firstInstance);
    }
    else
    {
        vkCmdDraw(cmd, vertexCount, instanceCount, firstOcurrence, firstInstance);
    }
}

void Context::upload_geometry(Buffer &vbo, size_t vboSize, const void *vboData, Buffer &ibo, size_t iboSize, const void *iboData, bool indexed)
{
    // Should be executed only once if geometry data is not changed

    Buffer vboStagingBuffer;
    vboStagingBuffer.init(memory, vboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    vboStagingBuffer.upload_data(memory, vboData, vboSize);

    // GPU vertex buffer
    vbo.init(memory, vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    uploadContext.immediate_submit(device, graphicsQueue, [=](VkCommandBuffer cmd)
                                   {
				VkBufferCopy copy;
				copy.dstOffset = 0;
				copy.srcOffset = 0;
				copy.size = vboSize;
				vkCmdCopyBuffer(cmd, vboStagingBuffer.buffer, vbo.buffer, 1, &copy); });

    vboStagingBuffer.cleanup(memory);

    if (indexed)
    {
        // Staging index buffer (CPU only)
        Buffer iboStagingBuffer;
        iboStagingBuffer.init(memory, iboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        iboStagingBuffer.upload_data(memory, iboData, iboSize);

        // GPU index buffer
        ibo.init(memory, iboSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        uploadContext.immediate_submit(device, graphicsQueue, [=](VkCommandBuffer cmd)
                                       {

					VkBufferCopy index_copy;
					index_copy.dstOffset = 0;
					index_copy.srcOffset = 0;
					index_copy.size = iboSize;
					vkCmdCopyBuffer(cmd, iboStagingBuffer.buffer, ibo.buffer, 1, &index_copy); });

        iboStagingBuffer.cleanup(memory);
    }
}
void Context::upload_texture_image(Image &img, const void *cache, VkFormat format, VkFilter filter, VkSamplerAddressMode adressMode, bool anisotropicFilter, bool useMipmaps)
{

    img.init(memory, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, img.extent, useMipmaps, VK_SAMPLE_COUNT_1_BIT);
    img.create_view(device, VK_IMAGE_ASPECT_COLOR_BIT);

    Buffer stagingBuffer;

    VkDeviceSize imageSize = img.extent.width * img.extent.height * img.extent.depth * Image::BYTES_PER_PIXEL;

    stagingBuffer.init(memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    stagingBuffer.upload_data(memory, cache, static_cast<size_t>(imageSize));

    uploadContext.immediate_submit(device, graphicsQueue, [&](VkCommandBuffer cmd)
                                   { img.upload_image(cmd, &stagingBuffer); });

    stagingBuffer.cleanup(memory);

    // GENERATE MIPMAPS
    if (img.mipLevels > 1)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, img.format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        uploadContext.immediate_submit(device, graphicsQueue, [&](VkCommandBuffer cmd)
                                       { img.generate_mipmaps(cmd); });
    }

    // CREATE SAMPLER
    img.create_sampler(device, filter,
                       VK_SAMPLER_MIPMAP_MODE_LINEAR,
                       adressMode,
                       0,
                       (float)img.mipLevels,
                       anisotropicFilter, utils::get_gpu_properties(gpu).limits.maxSamplerAnisotropy);
}

void Context::wait_for_device()
{
    VK_CHECK(vkDeviceWaitIdle(device));
}

VULKAN_ENGINE_NAMESPACE_END