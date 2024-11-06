/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CONTEXT_H
#define CONTEXT_H

#include <engine/common.h>

#include <engine/graphics/command_buffer.h>
#include <engine/graphics/extensions.h>
#include <engine/graphics/frame.h>
#include <engine/graphics/swapchain.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/utilities/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
/*
Vulkan API graphic context related data and functionality
*/
class Device
{
    VkDevice                               m_handle   = VK_NULL_HANDLE;
    VkInstance                             m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice                       m_gpu      = VK_NULL_HANDLE;
    VmaAllocator                           m_memory   = VK_NULL_HANDLE;
    Swapchain                              m_swapchain{};
    Utils::QueueFamilyIndices              m_queueFamilies{};
    std::unordered_map<QueueType, VkQueue> m_queues;
    DescriptorPool                         m_guiPool{};
    VkDebugUtilsMessengerEXT               m_debugMessenger = VK_NULL_HANDLE;
    Utils::UploadContext                   m_uploadContext{};
    const std::vector<const char*>         m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*>               m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                 "VK_EXT_extended_dynamic_state",
                                                                 VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                                                 VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                                                                 VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                                                 VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};
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
    inline VkDevice get_handle() const {
        return m_handle;
    };
    inline VmaAllocator get_memory_allocator() const {
        return m_memory;
    }
    inline VkPhysicalDevice get_GPU() const {
        return m_gpu;
    }
    inline Swapchain get_swapchain() const {
        return m_swapchain;
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

    void create_buffer(Buffer&               buffer,
                       size_t                allocSize,
                       VkBufferUsageFlags    usage,
                       VmaMemoryUsage        memoryUsage,
                       uint32_t              istrideSize,
                       std::vector<uint32_t> stridePartitionsSizes = {});
    void create_image(Image& image, bool useMipmaps, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);
    void create_frame(Frame& frame);
    void create_command_pool(CommandPool& pool, QueueType type);
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
    // void create_render_pass();
    // void create_graphic_pipeline();
    // void create_compute_pipeline();
    // void create_framebuffer();

    /*
    DRAWING
    -----------------------------------------------
    */

    VkResult    aquire_present_image(Frame& currentFrame, uint32_t& imageIndex);
    void        begin_command_buffer(Frame& currentFrame);
    void        end_command_buffer(Frame& currentFrame);
    VkResult    present_image(Frame& currentFrame, uint32_t imageIndex);
    static void draw_geometry(VkCommandBuffer& cmd,
                              VertexArrays&    vao,
                              uint32_t         instanceCount  = 1,
                              uint32_t         firstOcurrence = 0,
                              int32_t          offset         = 0,
                              uint32_t         firstInstance  = 0);
    static void draw_empty(VkCommandBuffer& cmd, uint32_t vertexCount = 0, uint32_t instanceCount = 1);

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
    void wait();
    void init_imgui(void*                 windowHandle,
                    WindowingSystem       windowingSystem,
                    VkRenderPass          renderPass,
                    VkSampleCountFlagBits samples);
    void destroy_imgui();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif