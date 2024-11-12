#include <engine/graphics/device.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void Device::init(void*            windowHandle,
                  WindowingSystem  windowingSystem,
                  VkExtent2D       surfaceExtent,
                  uint32_t         framesPerFlight,
                  VkFormat         presentFormat,
                  VkPresentModeKHR presentMode) {

    // BOOT Vulkan ------>>>

    // Instance
    m_instance = Booter::create_instance("xxx", "VulkanDevice", m_enableValidationLayers, m_validationLayers);
    if (m_enableValidationLayers)
        m_debugMessenger = Booter::create_debug_messenger(m_instance);
    // Surface
    Extent2D actualExtent = m_swapchain.create_surface(m_instance, windowHandle, windowingSystem);
    // Get gpu
    m_gpu = Booter::pick_graphics_card_device(m_instance, m_swapchain.get_surface(), m_deviceExtensions);
    // Create logical device
    m_handle = Booter::create_logical_device(m_queues,
                                             m_gpu,
                                             Utils::get_gpu_features(m_gpu),
                                             m_swapchain.get_surface(),
                                             m_enableValidationLayers,
                                             m_validationLayers);
    // Setup VMA
    m_memory = Booter::setup_memory(m_instance, m_handle, m_gpu);

    // Create swapchain
    m_swapchain.create(m_gpu, m_handle, actualExtent, surfaceExtent, framesPerFlight, presentFormat, presentMode);

    m_uploadContext.init(m_handle, m_gpu, m_swapchain.get_surface());

    // Load extension function pointers
    load_EXT_functions(m_handle);

    //------<<<
}

void Device::update_swapchain(VkExtent2D       surfaceExtent,
                              uint32_t         framesPerFlight,
                              VkFormat         presentFormat,
                              VkPresentModeKHR presentMode) {
    m_swapchain.create(m_gpu, m_handle, surfaceExtent, surfaceExtent, framesPerFlight, presentFormat, presentMode);
}

void Device::cleanup() {

    m_uploadContext.cleanup(m_handle);

    m_swapchain.cleanup();

    vmaDestroyAllocator(m_memory);

    vkDestroyDevice(m_handle, nullptr);

    if (m_enableValidationLayers)
    {
        Utils::destroy_debug_utils_messenger_EXT(m_instance, m_debugMessenger, nullptr);
    }

    m_swapchain.destroy_surface(m_instance);
    vkDestroyInstance(m_instance, nullptr);
}

void Device::create_buffer(Buffer&               buffer,
                           size_t                allocSize,
                           VkBufferUsageFlags    usage,
                           VmaMemoryUsage        memoryUsage,
                           uint32_t              istrideSize,
                           std::vector<uint32_t> stridePartitionsSizes) {
    buffer.init(m_memory, allocSize, usage, memoryUsage, istrideSize, stridePartitionsSizes);
}
void Device::create_image(Image& image, bool useMipmaps, VmaMemoryUsage memoryUsage) {
    image.init(m_handle, m_memory, useMipmaps, memoryUsage);
}
void Device::create_command_pool(CommandPool& pool, QueueType type) {
    pool.init(m_handle, Utils::find_queue_families(m_gpu, m_swapchain.get_surface()).graphicsFamily.value());
}
void Device::create_descriptor_pool(DescriptorPool& pool,
                                    uint32_t        maxSets,
                                    uint32_t        numUBO,
                                    uint32_t        numUBODynamic,
                                    uint32_t        numUBOStorage,
                                    uint32_t        numImageCombined,
                                    uint32_t        numSampler,
                                    uint32_t        numSampledImage,
                                    uint32_t        numStrgImage,
                                    uint32_t        numUBTexel,
                                    uint32_t        numStrgTexel,
                                    uint32_t        numUBOStorageDynamic,
                                    uint32_t        numIAttachment) {
    pool.init(m_handle,
              maxSets,
              numUBO,
              numUBODynamic,
              numUBOStorage,
              numImageCombined,
              numSampler,
              numSampledImage,
              numStrgImage,
              numUBTexel,
              numStrgTexel,
              numUBOStorageDynamic,
              numIAttachment);
}
void Device::create_render_pass(VulkanRenderPass&               rp,
                                std::vector<Attachment>&        attachments,
                                std::vector<SubPassDependency>& dependencies) {
    rp.init(m_handle, attachments, dependencies);
}
void Device::create_framebuffer(Framebuffer&             fbo,
                                VulkanRenderPass&        renderpass,
                                std::vector<Attachment>& attachments,
                                uint32_t                 layers) {
    fbo.init(renderpass, attachments, layers);
}

RenderResult Device::aquire_present_image(Semaphore& waitSemahpore, uint32_t& imageIndex) {
    VkResult result = vkAcquireNextImageKHR(
        m_handle, m_swapchain.get_handle(), UINT64_MAX, waitSemahpore.get_handle(), VK_NULL_HANDLE, &imageIndex);
    return static_cast<RenderResult>(result);
}

RenderResult Device::present_image(Semaphore& signalSemaphore, uint32_t imageIndex) {

    VkSemaphore      signalSemaphores[] = {signalSemaphore.get_handle()};
    VkPresentInfoKHR presentInfo        = Init::present_info();
    presentInfo.sType                   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount      = 1;
    presentInfo.pWaitSemaphores         = signalSemaphores;
    VkSwapchainKHR swapChains[]         = {m_swapchain.get_handle()};
    presentInfo.swapchainCount          = 1;
    presentInfo.pSwapchains             = swapChains;
    presentInfo.pImageIndices           = &imageIndex;

    VkResult result = vkQueuePresentKHR(m_queues[QueueType::PRESENT], &presentInfo);
    return static_cast<RenderResult>(result);
}

void Device::upload_vertex_arrays(VertexArrays& vao,
                                  size_t        vboSize,
                                  const void*   vboData,
                                  size_t        iboSize,
                                  const void*   iboData) {
    PROFILING_EVENT()
    // Should be executed only once if geometry data is not changed

    Buffer vboStagingBuffer;
    vboStagingBuffer.init(m_memory, vboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    vboStagingBuffer.upload_data(vboData, vboSize);

    // GPU vertex buffer
    vao.vbo.init(m_memory,
                 vboSize,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VMA_MEMORY_USAGE_GPU_ONLY);

    m_uploadContext.immediate_submit(m_handle, m_queues[QueueType::GRAPHIC], [=](VkCommandBuffer cmd) {
        VkBufferCopy copy;
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size      = vboSize;
        vkCmdCopyBuffer(cmd, vboStagingBuffer.get_handle(), vao.vbo.get_handle(), 1, &copy);
    });

    vboStagingBuffer.cleanup();

    if (vao.indexCount > 0)
    {
        // Staging index buffer (CPU only)
        Buffer iboStagingBuffer;
        iboStagingBuffer.init(m_memory, iboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        iboStagingBuffer.upload_data(iboData, iboSize);

        // GPU index buffer
        vao.ibo.init(m_memory,
                     iboSize,
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VMA_MEMORY_USAGE_GPU_ONLY);

        m_uploadContext.immediate_submit(m_handle, m_queues[QueueType::GRAPHIC], [=](VkCommandBuffer cmd) {
            VkBufferCopy index_copy;
            index_copy.dstOffset = 0;
            index_copy.srcOffset = 0;
            index_copy.size      = iboSize;
            vkCmdCopyBuffer(cmd, iboStagingBuffer.get_handle(), vao.ibo.get_handle(), 1, &index_copy);
        });

        iboStagingBuffer.cleanup();
    }
    vao.loadedOnGPU = true;
}
void Device::upload_texture_image(const void* imgCache, size_t bytesPerPixel, Image* const img, bool mipmapping) {
    PROFILING_EVENT()

    // CREATE IMAGE
    img->config.usageFlags =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    img->config.samples = VK_SAMPLE_COUNT_1_BIT;
    img->init(m_handle, m_memory, mipmapping);

    // CREATE VIEW
    img->viewConfig.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    // What if other type of texture
    img->create_view();

    Buffer stagingBuffer;

    VkDeviceSize imageSize = img->extent.width * img->extent.height * img->extent.depth * bytesPerPixel;

    stagingBuffer.init(m_memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    stagingBuffer.upload_data(imgCache, static_cast<size_t>(imageSize));

    m_uploadContext.immediate_submit(
        m_handle, m_queues[QueueType::GRAPHIC], [&](VkCommandBuffer cmd) { img->upload_image(cmd, &stagingBuffer); });

    stagingBuffer.cleanup();

    // GENERATE MIPMAPS
    if (img->config.mipLevels > 1)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_gpu, img->config.format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        m_uploadContext.immediate_submit(
            m_handle, m_queues[QueueType::GRAPHIC], [&](VkCommandBuffer cmd) { img->generate_mipmaps(cmd); });
    }

    // CREATE SAMPLER
    img->samplerConfig.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    img->samplerConfig.maxAnysotropy = Utils::get_gpu_properties(m_gpu).limits.maxSamplerAnisotropy;
    img->create_sampler();

    if (ImGui::GetCurrentContext())
        img->create_GUI_handle();

    img->loadedOnGPU = true;
}
void Device::wait() {
    VK_CHECK(vkDeviceWaitIdle(m_handle));
}

void Device::init_imgui(void*                 windowHandle,
                        WindowingSystem       windowingSystem,
                        VulkanRenderPass      renderPass,
                        VkSampleCountFlagBits samples) {

    m_guiPool.init(m_handle,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   1000,
                   VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(windowHandle), true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = m_instance;
    init_info.PhysicalDevice            = m_gpu;
    init_info.Device                    = m_handle;
    init_info.Queue                     = m_queues[QueueType::GRAPHIC];
    init_info.DescriptorPool            = m_guiPool.get_handle();
    init_info.MinImageCount             = 3;
    init_info.ImageCount                = 3;
    init_info.RenderPass                = renderPass.get_handle();
    init_info.MSAASamples               = samples;

    ImGui_ImplVulkan_Init(&init_info);
}
void Device::destroy_imgui() {
    ImGui_ImplVulkan_Shutdown();
    m_guiPool.cleanup();
} // namespace Graphics

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END