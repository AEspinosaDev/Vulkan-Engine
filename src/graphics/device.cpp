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
    m_allocator = Booter::setup_memory(m_instance, m_handle, m_gpu);

    // Create swapchain
    m_swapchain.create(m_gpu, m_handle, actualExtent, surfaceExtent, framesPerFlight, presentFormat, presentMode);

    m_uploadContext.init(m_handle, m_gpu, m_swapchain.get_surface());

    load_extensions(m_handle);

    // Get properties
    vkGetPhysicalDeviceProperties(m_gpu, &m_properties);
    vkGetPhysicalDeviceFeatures(m_gpu, &m_features);
    vkGetPhysicalDeviceMemoryProperties(m_gpu, &m_memoryProperties);
    // uint32_t queueFamilyCount;
    // vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queueFamilyCount, nullptr);
    // assert(queueFamilyCount > 0);
    // queueFamilyProperties.resize(queueFamilyCount);
    // vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queueFamilyCount, queueFamilyProperties.data());

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

    vmaDestroyAllocator(m_allocator);

    vkDestroyDevice(m_handle, nullptr);

    if (m_enableValidationLayers)
    {
        Utils::destroy_debug_utils_messenger_EXT(m_instance, m_debugMessenger, nullptr);
    }

    m_swapchain.destroy_surface(m_instance);
    vkDestroyInstance(m_instance, nullptr);
}

Buffer
Device::create_buffer_VMA(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t strideSize) {

    Buffer buffer = {};

    VkBufferCreateInfo bufferInfo        = {};
    bufferInfo.sType                     = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext                     = nullptr;
    bufferInfo.size                      = allocSize;
    bufferInfo.usage                     = usage;
    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage                   = memoryUsage;

    VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &vmaallocInfo, &buffer.handle, &buffer.allocation, nullptr));

    buffer.device     = m_handle;
    buffer.allocator  = m_allocator;
    buffer.size       = allocSize;
    buffer.strideSize = strideSize == 0 ? allocSize : strideSize;

    return buffer;
}
Buffer Device::create_buffer(size_t                allocSize,
                             VkBufferUsageFlags    usage,
                             VkMemoryPropertyFlags memoryProperties,
                             uint32_t              strideSize) {
    Buffer buffer = {};

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size  = allocSize;
    bufferCreateInfo.usage = usage;
    VK_CHECK(vkCreateBuffer(m_handle, &bufferCreateInfo, nullptr, &buffer.handle));

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(m_handle, buffer.handle, &memoryRequirements);
    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext           = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize  = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = get_memory_type(memoryRequirements.memoryTypeBits, memoryProperties);

    VK_CHECK(vkAllocateMemory(m_handle, &memoryAllocateInfo, nullptr, &buffer.memory));
    VK_CHECK(vkBindBufferMemory(m_handle, buffer.handle, buffer.memory, 0));

    buffer.device     = m_handle;
    buffer.size       = allocSize;
    buffer.strideSize = strideSize == 0 ? allocSize : strideSize;

    if ((memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
        buffer.coherence = true;

    return buffer;
}
void Device::create_image(Image& image, bool useMipmaps, VmaMemoryUsage memoryUsage) {
    image.init(m_handle, m_allocator, useMipmaps, memoryUsage);
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
                                Extent2D                 extent,
                                VulkanRenderPass&        renderpass,
                                std::vector<Attachment>& attachments,
                                uint32_t                 layers) {
    fbo.init(renderpass, extent, attachments, layers);
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

    Buffer vboStagingBuffer = create_buffer_VMA(vboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    vboStagingBuffer.upload_data(vboData, vboSize);

    // GPU vertex buffer
    vao.vbo = create_buffer_VMA(vboSize,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                    VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                VMA_MEMORY_USAGE_GPU_ONLY);

    m_uploadContext.immediate_submit(m_handle, m_queues[QueueType::GRAPHIC], [&](VkCommandBuffer cmd) {
        VkBufferCopy copy;
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size      = vboSize;
        vkCmdCopyBuffer(cmd, vboStagingBuffer.handle, vao.vbo.handle, 1, &copy);
    });

    vboStagingBuffer.cleanup();

    if (vao.indexCount > 0)
    {
        // Staging index buffer (CPU only)
        Buffer iboStagingBuffer =
            create_buffer_VMA(iboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        iboStagingBuffer.upload_data(iboData, iboSize);

        // GPU index buffer
        vao.ibo = create_buffer_VMA(iboSize,
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                    VMA_MEMORY_USAGE_GPU_ONLY);

        m_uploadContext.immediate_submit(m_handle, m_queues[QueueType::GRAPHIC], [&](VkCommandBuffer cmd) {
            VkBufferCopy index_copy;
            index_copy.dstOffset = 0;
            index_copy.srcOffset = 0;
            index_copy.size      = iboSize;
            vkCmdCopyBuffer(cmd, iboStagingBuffer.handle, vao.ibo.handle, 1, &index_copy);
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
    img->init(m_handle, m_allocator, mipmapping);

    // CREATE VIEW
    img->viewConfig.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    // What if other type of texture
    img->create_view();

    VkDeviceSize imageSize = img->extent.width * img->extent.height * img->extent.depth * bytesPerPixel;

    Buffer stagingBuffer = create_buffer_VMA(imageSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
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
void Device::create_BLAS(BLAS& accel, VAO& vao) {
    if (!vao.loadedOnGPU)
        return;

    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

    vertexBufferDeviceAddress.deviceAddress = vao.vbo.get_device_address();
    if (vao.indexCount > 0)
        indexBufferDeviceAddress.deviceAddress = vao.ibo.get_device_address();

    // GEOMETRY
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry = Init::acceleration_structure_geometry();
    accelerationStructureGeometry.flags                              = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometryType                       = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    accelerationStructureGeometry.geometry.triangles.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    accelerationStructureGeometry.geometry.triangles.vertexData   = vertexBufferDeviceAddress;
    accelerationStructureGeometry.geometry.triangles.maxVertex    = vao.vertexCount - 1;
    accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Utils::Vertex);

    if (vao.indexCount > 0)
    {
        accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
    }
    accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
    accelerationStructureGeometry.geometry.triangles.transformData.hostAddress   = nullptr;

    // SIZE INFO
    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo =
        Init::acceleration_structure_build_geometry_info();
    accelerationStructureBuildGeometryInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries   = &accelerationStructureGeometry;

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo =
        Init::acceleration_structure_build_sizes_info();

    const uint32_t numTriangles = 1;
    vkGetAccelerationStructureBuildSizes(m_handle,
                                         VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                         &accelerationStructureBuildGeometryInfo,
                                         &numTriangles,
                                         &accelerationStructureBuildSizesInfo);

    // CREATE ACCELERATION BUFFER
    accel.buffer = create_buffer(
        accelerationStructureBuildSizesInfo.accelerationStructureSize,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // CREATE ACCELERATION STRUCTURE
    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
    accelerationStructureCreateInfo.sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.buffer = accel.buffer.handle;
    accelerationStructureCreateInfo.size   = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    accelerationStructureCreateInfo.type   = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    if (vkCreateAccelerationStructure(m_handle, &accelerationStructureCreateInfo, nullptr, &accel.handle) != VK_SUCCESS)
    {
        throw VKFW_Exception("Failed to create BLAS!");
    }

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    Buffer scratchBuffer = create_buffer(accel.buffer.size,
                                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
    accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationBuildGeometryInfo.type  = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode  = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure  = accel.handle;
    accelerationBuildGeometryInfo.geometryCount             = 1;
    accelerationBuildGeometryInfo.pGeometries               = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.get_device_address();

    VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
    accelerationStructureBuildRangeInfo.primitiveCount                                          = numTriangles;
    accelerationStructureBuildRangeInfo.primitiveOffset                                         = 0;
    accelerationStructureBuildRangeInfo.firstVertex                                             = 0;
    accelerationStructureBuildRangeInfo.transformOffset                                         = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = {
        &accelerationStructureBuildRangeInfo};

    m_uploadContext.immediate_submit(m_handle, m_queues[QueueType::GRAPHIC], [&](VkCommandBuffer cmd) {
        vkCmdBuildAccelerationStructures(
            cmd, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
    });

    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
    accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure = accel.handle;
    accel.deviceAdress = vkGetAccelerationStructureDeviceAddress(m_handle, &accelerationDeviceAddressInfo);

    scratchBuffer.cleanup();

    accel.device = m_handle;
}

void Device::create_TLAS(TLAS& accel, std::vector<BLAS>& blases) {

    // Set up instance data for each BLAS
    VkTransformMatrixKHR transformMatrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    std::vector<VkAccelerationStructureInstanceKHR> instances;
    instances.resize(blases.size(), {});

    for (size_t i = 0; i < blases.size(); ++i)
    {
        instances[i].transform                              = transformMatrix;
        instances[i].instanceCustomIndex                    = i;
        instances[i].mask                                   = 0xFF;
        instances[i].instanceShaderBindingTableRecordOffset = 0;
        instances[i].flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instances[i].accelerationStructureReference         = blases[i].deviceAdress;
    }

    // Create a buffer for the instances
    Buffer instanceBuffer = create_buffer(sizeof(VkAccelerationStructureInstanceKHR) * instances.size(),
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                                              VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    instanceBuffer.upload_data(instances.data(), sizeof(VkAccelerationStructureInstanceKHR) * instances.size());

    VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
    instanceDataDeviceAddress.deviceAddress = instanceBuffer.get_device_address();

    VkAccelerationStructureGeometryKHR accelerationStructureGeometry = Init::acceleration_structure_geometry();
    accelerationStructureGeometry.geometryType                       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    accelerationStructureGeometry.flags                              = VK_GEOMETRY_OPAQUE_BIT_KHR;
    accelerationStructureGeometry.geometry.instances.sType =
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
    accelerationStructureGeometry.geometry.instances.data            = instanceDataDeviceAddress;

    VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo =
        Init::acceleration_structure_build_geometry_info();
    accelerationStructureBuildGeometryInfo.type          = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationStructureBuildGeometryInfo.flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationStructureBuildGeometryInfo.geometryCount = 1;
    accelerationStructureBuildGeometryInfo.pGeometries   = &accelerationStructureGeometry;

    uint32_t primitiveCount = static_cast<uint32_t>(instances.size());

    VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo =
        Init::acceleration_structure_build_sizes_info();
    vkGetAccelerationStructureBuildSizes(m_handle,
                                            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                            &accelerationStructureBuildGeometryInfo,
                                            &primitiveCount,
                                            &accelerationStructureBuildSizesInfo);

    // CREATE ACCELERATION BUFFER
    accel.buffer = create_buffer(
        accelerationStructureBuildSizesInfo.accelerationStructureSize,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
    accelerationStructureCreateInfo.sType  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationStructureCreateInfo.buffer = accel.buffer.handle;
    accelerationStructureCreateInfo.size   = accelerationStructureBuildSizesInfo.accelerationStructureSize;
    accelerationStructureCreateInfo.type   = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    if (vkCreateAccelerationStructure(m_handle, &accelerationStructureCreateInfo, nullptr, &accel.handle) != VK_SUCCESS)
    {
        throw VKFW_Exception("Failed to create TLAS!");
    };

    // Create a small scratch buffer used during build of the bottom level acceleration structure
    Buffer scratchBuffer = create_buffer(accelerationStructureBuildSizesInfo.buildScratchSize,
                                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo =
        Init::acceleration_structure_build_geometry_info();
    accelerationBuildGeometryInfo.type                      = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationBuildGeometryInfo.flags                     = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    accelerationBuildGeometryInfo.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure  = accel.handle;
    accelerationBuildGeometryInfo.geometryCount             = 1;
    accelerationBuildGeometryInfo.pGeometries               = &accelerationStructureGeometry;
    accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.get_device_address();

    VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
    accelerationStructureBuildRangeInfo.primitiveCount                                          = 1;
    accelerationStructureBuildRangeInfo.primitiveOffset                                         = 0;
    accelerationStructureBuildRangeInfo.firstVertex                                             = 0;
    accelerationStructureBuildRangeInfo.transformOffset                                         = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = {
        &accelerationStructureBuildRangeInfo};

    m_uploadContext.immediate_submit(m_handle, m_queues[QueueType::GRAPHIC], [&](VkCommandBuffer cmd) {
        vkCmdBuildAccelerationStructures(
            cmd, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos.data());
    });

    VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
    accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    accelerationDeviceAddressInfo.accelerationStructure = accel.handle;
    accel.deviceAdress = vkGetAccelerationStructureDeviceAddress(m_handle, &accelerationDeviceAddressInfo);

    scratchBuffer.cleanup();
    instanceBuffer.cleanup();

    accel.device = m_handle;
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

uint32_t Device::get_memory_type(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) {
    for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                if (memTypeFound)
                {
                    *memTypeFound = true;
                }
                return i;
            }
        }
        typeBits >>= 1;
    }

    if (memTypeFound)
    {
        *memTypeFound = false;
        return 0;
    } else
    {
        throw std::runtime_error("Could not find a matching memory type");
    }
}
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END