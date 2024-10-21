#include <engine/graphics/context.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

void Context::init(void *windowHandle, WindowingSystem windowingSystem, VkExtent2D surfaceExtent,
                   uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode)
{

    // BOOT Vulkan ------>>>

    VKBooter booter(enableValidationLayers);

    instance = booter.boot_vulkan();
    debugMessenger = booter.create_debug_messenger(instance);

    VkExtent2D actualExtent{};
    if (windowingSystem == WindowingSystem::GLFW)
    {
        GLFWwindow *glfwHandle = static_cast<GLFWwindow *>(windowHandle);
        VK_CHECK(glfwCreateWindowSurface(instance, glfwHandle, nullptr, &surface));
        int width, height;
        glfwGetFramebufferSize(glfwHandle, &width, &height);
        actualExtent = {static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
    }
    else
    {
        // TO DO SDL .. 
    }

    // Get gpu
    gpu = booter.pick_graphics_card_device(instance, surface);

    // Create logical device
    device = booter.create_logical_device(graphicsQueue, presentQueue, gpu, utils::get_gpu_features(gpu), surface);

    // Setup VMA
    memory = booter.setup_memory(instance, device, gpu);

    uploadContext.init(device, gpu, surface);

    // Create swapchain
    swapchain.create(gpu, device, surface, actualExtent, surfaceExtent, framesPerFlight, presentFormat, presentMode);

    // Init frames with control objects
    frames.resize(framesPerFlight);
    for (size_t i = 0; i < frames.size(); i++)
        frames[i].init(device, gpu, surface);

    // Load extension function pointers
    load_EXT_functions(device);
    //------<<<
}

void Context::update_swapchain(VkExtent2D surfaceExtent,
                               uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode)
{
    swapchain.create(gpu, device, surface, surfaceExtent, surfaceExtent, framesPerFlight, presentFormat, presentMode);
}

void Context::cleanup()
{
    for (size_t i = 0; i < frames.size(); i++)
    {
        frames[i].cleanup(device);
    }

    uploadContext.cleanup(device);

    swapchain.cleanup(device);

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
    VkResult result = vkAcquireNextImageKHR(device, swapchain.get_handle(), UINT64_MAX,
                                            frames[currentFrame].presentSemaphore, VK_NULL_HANDLE, &imageIndex);
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

void Context::draw_geometry(VkCommandBuffer &cmd, Buffer &vbo, Buffer &ibo, uint32_t vertexCount, uint32_t indexCount,
                            bool indexed, uint32_t instanceCount, uint32_t firstOcurrence, int32_t offset,
                            uint32_t firstInstance)
{
    PROFILING_EVENT()

    VkBuffer vertexBuffers[] = {vbo.handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    if (indexed)
    {
        vkCmdBindIndexBuffer(cmd, ibo.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstOcurrence, offset, firstInstance);
    }
    else
    {
        vkCmdDraw(cmd, vertexCount, instanceCount, firstOcurrence, firstInstance);
    }
}

void Context::upload_geometry(Buffer &vbo, size_t vboSize, const void *vboData, Buffer &ibo, size_t iboSize,
                              const void *iboData, bool indexed)
{
    PROFILING_EVENT()
    // Should be executed only once if geometry data is not changed

    Buffer vboStagingBuffer;
    vboStagingBuffer.init(memory, vboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    vboStagingBuffer.upload_data(memory, vboData, vboSize);

    // GPU vertex buffer
    vbo.init(memory, vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
             VMA_MEMORY_USAGE_GPU_ONLY);

    uploadContext.immediate_submit(device, graphicsQueue, [=](VkCommandBuffer cmd) {
        VkBufferCopy copy;
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size = vboSize;
        vkCmdCopyBuffer(cmd, vboStagingBuffer.handle, vbo.handle, 1, &copy);
    });

    vboStagingBuffer.cleanup(memory);

    if (indexed)
    {
        // Staging index buffer (CPU only)
        Buffer iboStagingBuffer;
        iboStagingBuffer.init(memory, iboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        iboStagingBuffer.upload_data(memory, iboData, iboSize);

        // GPU index buffer
        ibo.init(memory, iboSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VMA_MEMORY_USAGE_GPU_ONLY);

        uploadContext.immediate_submit(device, graphicsQueue, [=](VkCommandBuffer cmd) {
            VkBufferCopy index_copy;
            index_copy.dstOffset = 0;
            index_copy.srcOffset = 0;
            index_copy.size = iboSize;
            vkCmdCopyBuffer(cmd, iboStagingBuffer.handle, ibo.handle, 1, &index_copy);
        });

        iboStagingBuffer.cleanup(memory);
    }
}
void Context::upload_texture_image(Image *const img, bool mipmapping)
{
    PROFILING_EVENT()

    // CREATE IMAGE
    img->config.usageFlags =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    img->config.samples = VK_SAMPLE_COUNT_1_BIT;
    img->init(memory, mipmapping);

    // CREATE VIEW
    img->viewConfig.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    // What if other type of texture
    img->create_view(device);

    Buffer stagingBuffer;

    VkDeviceSize imageSize = img->extent.width * img->extent.height * img->extent.depth * Image::BYTES_PER_PIXEL;

    stagingBuffer.init(memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    stagingBuffer.upload_data(memory, static_cast<const void *>(img->tmpCache), static_cast<size_t>(imageSize));

    uploadContext.immediate_submit(device, graphicsQueue,
                                   [&](VkCommandBuffer cmd) { img->upload_image(cmd, &stagingBuffer); });

    stagingBuffer.cleanup(memory);

    // GENERATE MIPMAPS
    if (img->config.mipLevels > 1)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gpu, img->config.format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        uploadContext.immediate_submit(device, graphicsQueue, [&](VkCommandBuffer cmd) { img->generate_mipmaps(cmd); });
    }

    // CREATE SAMPLER
    img->samplerConfig.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    img->samplerConfig.maxAnysotropy = utils::get_gpu_properties(gpu).limits.maxSamplerAnisotropy;
    img->create_sampler(device);

    img->loadedOnGPU = true;
}

void Context::wait_for_device()
{
    VK_CHECK(vkDeviceWaitIdle(device));
}

void Context::init_gui_pool()
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

    VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &m_guiPool));
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END