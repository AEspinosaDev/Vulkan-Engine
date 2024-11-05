#include <engine/graphics/device.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

void Device::init(void *windowHandle, WindowingSystem windowingSystem, VkExtent2D surfaceExtent,
                  uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode)
{

    // BOOT Vulkan ------>>>

    // Instance
    instance = Booter::create_instance("xxx", "VulkanDevice", enableValidationLayers, m_validationLayers);
    if (enableValidationLayers)
        debugMessenger = Booter::create_debug_messenger(instance);
    // Surface
    Extent2D actualExtent = swapchain.create_surface(instance, windowHandle, windowingSystem);
    // Get gpu
    gpu = Booter::pick_graphics_card_device(instance, swapchain.get_surface(), m_deviceExtensions);
    // Create logical device
    handle = Booter::create_logical_device(queues, gpu, Utils::get_gpu_features(gpu),
                                           swapchain.get_surface(), enableValidationLayers, m_validationLayers);
    // Setup VMA
    memory = Booter::setup_memory(instance, handle, gpu);

    // Create swapchain
    swapchain.create(gpu, handle, actualExtent, surfaceExtent, framesPerFlight, presentFormat, presentMode);

    uploadContext.init(handle, gpu, swapchain.get_surface());

    // Init frames with control objects
    frames.resize(framesPerFlight);
    for (size_t i = 0; i < frames.size(); i++)
        frames[i].init(handle, gpu, swapchain.get_surface());

    // Load extension function pointers
    load_EXT_functions(handle);

    //------<<<
}

void Device::update_swapchain(VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat,
                              VkPresentModeKHR presentMode)
{
    swapchain.create(gpu, handle, surfaceExtent, surfaceExtent, framesPerFlight, presentFormat, presentMode);
}

void Device::cleanup()
{
    for (size_t i = 0; i < frames.size(); i++)
    {
        frames[i].cleanup(handle);
    }

    uploadContext.cleanup(handle);

    swapchain.cleanup(handle);

    vmaDestroyAllocator(memory);

    vkDestroyDevice(handle, nullptr);

    if (enableValidationLayers)
    {
        Utils::destroy_debug_utils_messenger_EXT(instance, debugMessenger, nullptr);
    }

    swapchain.destroy_surface(instance);
    vkDestroyInstance(instance, nullptr);
}

void Device::init_buffer(Buffer &buffer, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                         uint32_t istrideSize, std::vector<uint32_t> stridePartitionsSizes)
{
    buffer.init(memory, allocSize, usage, memoryUsage, istrideSize, stridePartitionsSizes);
}
void Device::init_image(Image &image, bool useMipmaps, VmaMemoryUsage memoryUsage)
{
    image.init(handle, memory, useMipmaps, memoryUsage);
}
void Device::init_command_pool(CommandPool &pool, QueueType type)
{
    pool.init(handle, Utils::find_queue_families(gpu, swapchain.get_surface()).graphicsFamily.value());
}
VkResult Device::aquire_present_image(const uint32_t &currentFrame, uint32_t &imageIndex)
{
    VK_CHECK(vkWaitForFences(handle, 1, &frames[currentFrame].renderFence, VK_TRUE, UINT64_MAX));
    VkResult result = vkAcquireNextImageKHR(handle, swapchain.get_handle(), UINT64_MAX,
                                            frames[currentFrame].presentSemaphore, VK_NULL_HANDLE, &imageIndex);
    return result;
}

void Device::begin_command_buffer(const uint32_t &currentFrame)
{
    VK_CHECK(vkResetFences(handle, 1, &frames[currentFrame].renderFence));
    VK_CHECK(vkResetCommandBuffer(frames[currentFrame].commandBuffer, 0));

    VkCommandBufferBeginInfo beginInfo = Init::command_buffer_begin_info();

    if (vkBeginCommandBuffer(frames[currentFrame].commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw VKException("failed to begin recording command buffer!");
    }
}

void Device::end_command_buffer(const uint32_t &currentFrame)
{
    if (vkEndCommandBuffer(frames[currentFrame].commandBuffer) != VK_SUCCESS)
    {
        throw VKException("failed to record command buffer!");
    }
}

VkResult Device::present_image(const uint32_t &currentFrame, uint32_t imageIndex)
{
    VkSubmitInfo submitInfo = Init::submit_info(&frames[currentFrame].commandBuffer);
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

    if (vkQueueSubmit(queues[QueueType::GRAPHIC], 1, &submitInfo, frames[currentFrame].renderFence) != VK_SUCCESS)
    {
        throw VKException("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = Init::present_info();
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchain.get_handle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(queues[QueueType::PRESENT], &presentInfo);
}

void Device::draw_geometry(VkCommandBuffer &cmd, Buffer &vbo, Buffer &ibo, uint32_t vertexCount, uint32_t indexCount,
                           bool indexed, uint32_t instanceCount, uint32_t firstOcurrence, int32_t offset,
                           uint32_t firstInstance)
{
    PROFILING_EVENT()

    VkBuffer vertexBuffers[] = {vbo.get_handle()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    if (indexed)
    {
        vkCmdBindIndexBuffer(cmd, ibo.get_handle(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstOcurrence, offset, firstInstance);
    }
    else
    {
        vkCmdDraw(cmd, vertexCount, instanceCount, firstOcurrence, firstInstance);
    }
}

void Device::draw_empty(VkCommandBuffer &cmd, uint32_t vertexCount, uint32_t instanceCount)
{
    PROFILING_EVENT()
    vkCmdDraw(cmd, vertexCount, instanceCount, 0, 0);
}
void Device::upload_vertex_arrays(VertexArrays &vao, size_t vboSize, const void *vboData, size_t iboSize,
                                  const void *iboData)
{
    PROFILING_EVENT()
    // Should be executed only once if geometry data is not changed

    Buffer vboStagingBuffer;
    vboStagingBuffer.init(memory, vboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    vboStagingBuffer.upload_data(vboData, vboSize);

    // GPU vertex buffer
    vao.vbo.init(memory, vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VMA_MEMORY_USAGE_GPU_ONLY);

    uploadContext.immediate_submit(handle, queues[QueueType::GRAPHIC], [=](VkCommandBuffer cmd) {
        VkBufferCopy copy;
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size = vboSize;
        vkCmdCopyBuffer(cmd, vboStagingBuffer.get_handle(), vao.vbo.get_handle(), 1, &copy);
    });

    vboStagingBuffer.cleanup();

    if (vao.indexCount > 0)
    {
        // Staging index buffer (CPU only)
        Buffer iboStagingBuffer;
        iboStagingBuffer.init(memory, iboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        iboStagingBuffer.upload_data(iboData, iboSize);

        // GPU index buffer
        vao.ibo.init(memory, iboSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VMA_MEMORY_USAGE_GPU_ONLY);

        uploadContext.immediate_submit(handle, queues[QueueType::GRAPHIC], [=](VkCommandBuffer cmd) {
            VkBufferCopy index_copy;
            index_copy.dstOffset = 0;
            index_copy.srcOffset = 0;
            index_copy.size = iboSize;
            vkCmdCopyBuffer(cmd, iboStagingBuffer.get_handle(), vao.ibo.get_handle(), 1, &index_copy);
        });

        iboStagingBuffer.cleanup();
    }
    vao.loadedOnGPU = true;
}
void Device::destroy_vertex_arrays(VertexArrays &vao)
{
    vao.vbo.cleanup();
    if (vao.indexCount > 0)
        vao.ibo.cleanup();
}
void Device::upload_texture_image(const void *imgCache, size_t bytesPerPixel, Image *const img, bool mipmapping)
{
    PROFILING_EVENT()

    // CREATE IMAGE
    img->config.usageFlags =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    img->config.samples = VK_SAMPLE_COUNT_1_BIT;
    img->init(handle, memory, mipmapping);

    // CREATE VIEW
    img->viewConfig.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    // What if other type of texture
    img->create_view();

    Buffer stagingBuffer;

    VkDeviceSize imageSize = img->extent.width * img->extent.height * img->extent.depth * bytesPerPixel;

    stagingBuffer.init(memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    stagingBuffer.upload_data(imgCache, static_cast<size_t>(imageSize));

    uploadContext.immediate_submit(handle, queues[QueueType::GRAPHIC],
                                   [&](VkCommandBuffer cmd) { img->upload_image(cmd, &stagingBuffer); });

    stagingBuffer.cleanup();

    // GENERATE MIPMAPS
    if (img->config.mipLevels > 1)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, img->config.format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        uploadContext.immediate_submit(handle, queues[QueueType::GRAPHIC],
                                       [&](VkCommandBuffer cmd) { img->generate_mipmaps(cmd); });
    }

    // CREATE SAMPLER
    img->samplerConfig.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    img->samplerConfig.maxAnysotropy = Utils::get_gpu_properties(gpu).limits.maxSamplerAnisotropy;
    img->create_sampler();

    img->loadedOnGPU = true;
}

void Device::destroy_texture_image(Image *const img)
{
    img->cleanup();
}

void Device::wait()
{
    VK_CHECK(vkDeviceWaitIdle(handle));
}

void Device::init_gui_pool()
{
    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VK_CHECK(vkCreateDescriptorPool(handle, &pool_info, nullptr, &m_guiPool));
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END