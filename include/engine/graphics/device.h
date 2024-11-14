/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CONTEXT_H
#define CONTEXT_H

#include <engine/common.h>

#include <engine/graphics/accel.h>
#include <engine/graphics/command_buffer.h>
#include <engine/graphics/extensions.h>
#include <engine/graphics/frame.h>
#include <engine/graphics/framebuffer.h>
#include <engine/graphics/swapchain.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/utilities/utils.h>
#include <engine/graphics/vk_renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
/*
Vulkan API graphic context related data and functionality
*/
class Device
{
    // Main items
    VkDevice                               m_handle        = VK_NULL_HANDLE;
    VkInstance                             m_instance      = VK_NULL_HANDLE;
    VkPhysicalDevice                       m_gpu           = VK_NULL_HANDLE;
    VmaAllocator                           m_allocator     = VK_NULL_HANDLE;
    Swapchain                              m_swapchain     = {};
    Utils::QueueFamilyIndices              m_queueFamilies = {};
    DescriptorPool                         m_guiPool       = {};
    Utils::UploadContext                   m_uploadContext = {};
    std::unordered_map<QueueType, VkQueue> m_queues;
    // GPU Properties
    VkPhysicalDeviceProperties       m_properties       = {};
    VkPhysicalDeviceFeatures         m_features         = {};
    VkPhysicalDeviceFeatures         m_enabledFeatures  = {};
    VkPhysicalDeviceMemoryProperties m_memoryProperties = {};
    // std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    // Validation
    VkDebugUtilsMessengerEXT       m_debugMessenger   = VK_NULL_HANDLE;
    const std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
    // Extensions
    std::vector<const char*> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                   "VK_EXT_extended_dynamic_state",
                                                   VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                                   VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                                                   VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                                   VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                                   VK_KHR_RAY_QUERY_EXTENSION_NAME};
#ifdef NDEBUG
    const bool m_enableValidationLayers{false};
#else
    const bool m_enableValidationLayers{true};
#endif

  public:
    /*
    GETTERS
    -----------------------------------------------
    */
    inline VkDevice& get_handle() {
        return m_handle;
    };
    inline VmaAllocator& get_memory_allocator() {
        return m_allocator;
    }
    inline VkPhysicalDevice& get_GPU() {
        return m_gpu;
    }
    inline Swapchain get_swapchain() const {
        return m_swapchain;
    }
    inline std::unordered_map<QueueType, VkQueue>& get_queues() {
        return m_queues;
    }

    /*
    INIT AND SHUTDOWN
    -----------------------------------------------
    */
    void init(void*            windowHandle,
              WindowingSystem  windowingSystem,
              VkExtent2D       surfaceExtent,
              uint32_t         framesPerFlight,
              VkFormat         presentFormat,
              VkPresentModeKHR presentMode);
    void update_swapchain(VkExtent2D       surfaceExtent,
                          uint32_t         framesPerFlight,
                          VkFormat         presentFormat,
                          VkPresentModeKHR presentMode);
    void cleanup();

    /*
    OBJECT CREATION
    -----------------------------------------------
    */

    /*Create Buffer using Vulkan Memory Allocator (VMA)*/
    Buffer
    create_buffer_VMA(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t strideSize = 0);
    /*Create Buffer*/
    Buffer create_buffer(size_t                allocSize,
                         VkBufferUsageFlags    usage,
                         VkMemoryPropertyFlags memoryProperties,
                         uint32_t              strideSize = 0);
    /*Create Image*/
    void create_image(Image& image, bool useMipmaps, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);
    /*Create Command Pool*/
    void create_command_pool(CommandPool& pool, QueueType type);
    /*Create RenderPass*/
    void create_render_pass(VulkanRenderPass&               rp,
                            std::vector<Attachment>&        attachments,
                            std::vector<SubPassDependency>& dependencies);
    /*Create Framebuffer Object*/
    void create_framebuffer(Framebuffer&             fbo,
                            Extent2D                 extent,
                            VulkanRenderPass&        renderpass,
                            std::vector<Attachment>& attachments,
                            uint32_t                 layers = 1);
    /*Create and Setup Bottom-Level Acceleration Structure*/
    void create_BLAS(BLAS& accel, VAO& vao);
    /*Create and Setup Top-Level Acceleration Structure*/
    void create_TLAS(TLAS& accel, std::vector<BLASInstance>& BLASinstances);
    /*Create Descriptor Pool*/
    void create_descriptor_pool(DescriptorPool& pool,
                                uint32_t        maxSets,
                                uint32_t        numUBO,
                                uint32_t        numUBODynamic,
                                uint32_t        numUBOStorage,
                                uint32_t        numImageCombined,
                                uint32_t        numSampler           = 0,
                                uint32_t        numSampledImage      = 0,
                                uint32_t        numStrgImage         = 0,
                                uint32_t        numUBTexel           = 0,
                                uint32_t        numStrgTexel         = 0,
                                uint32_t        numUBOStorageDynamic = 0,
                                uint32_t        numIAttachment       = 0);

    /*
    DRAWING
    -----------------------------------------------
    */
    RenderResult prepare_frame(Frame& frame, uint32_t& imageIndex) {
    }
    RenderResult submit_frame(Frame& frame, uint32_t imageIndex) {
    }
    RenderResult aquire_present_image(Semaphore& waitSemahpore, uint32_t& imageIndex);
    RenderResult present_image(Semaphore& signalSemaphore, uint32_t imageIndex);

    /*
    DATA TRANSFER
    -----------------------------------------------
    */
    void
    upload_vertex_arrays(VertexArrays& vao, size_t vboSize, const void* vboData, size_t iboSize, const void* iboData);
    void upload_texture_image(const void* imgCache, size_t bytesPerPixel, Image* const img, bool mipmapping);

    /*
    MISC
    -----------------------------------------------
    */
    void     wait();
    void     init_imgui(void*                 windowHandle,
                        WindowingSystem       windowingSystem,
                        VulkanRenderPass      renderPass,
                        VkSampleCountFlagBits samples);
    void     destroy_imgui();
    uint32_t get_memory_type(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif