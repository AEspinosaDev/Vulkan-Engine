/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CONTEXT_H
#define CONTEXT_H

#include <engine/common.h>
#include <engine/utils.h>

#include <engine/graphics/accel.h>
#include <engine/graphics/command_buffer.h>
#include <engine/graphics/descriptors.h>
#include <engine/graphics/extensions.h>
#include <engine/graphics/framebuffer.h>
#include <engine/graphics/renderpass.h>
#include <engine/graphics/swapchain.h>
#include <engine/graphics/texture.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
/*
Vulkan API graphic context related data and functionality
*/
class Device
{
    // Main items
    VkDevice                               m_handle    = VK_NULL_HANDLE;
    VkInstance                             m_instance  = VK_NULL_HANDLE;
    VkPhysicalDevice                       m_gpu       = VK_NULL_HANDLE;
    VmaAllocator                           m_allocator = VK_NULL_HANDLE;
    Swapchain                              m_swapchain = {};
    DescriptorPool                         m_guiPool   = {};
    std::unordered_map<QueueType, VkQueue> m_queues;
    // GPU Properties
    VkPhysicalDeviceProperties       m_properties       = {};
    VkPhysicalDeviceFeatures         m_features         = {};
    VkPhysicalDeviceMemoryProperties m_memoryProperties = {};
    // Validation
    VkDebugUtilsMessengerEXT       m_debugMessenger   = VK_NULL_HANDLE;
    const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
    // Extensions
    std::vector<const char*> m_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                              "VK_EXT_extended_dynamic_state",
                                              VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                              VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                                              VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                              VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                              VK_KHR_RAY_QUERY_EXTENSION_NAME };
    // Utils
    struct UploadContext {
        Fence         uploadFence;
        CommandPool   commandPool;
        CommandBuffer commandBuffer;

        void immediate_submit( std::function<void( CommandBuffer cmd )>&& function );
        void cleanup();
    };
    UploadContext m_uploadContext = {};

#ifdef NDEBUG
    const bool m_enableValidationLayers { false };
#else
    const bool m_enableValidationLayers { true };
#endif

    void create_upload_context();

public:
    /*
    GETTERS
    -----------------------------------------------
    */
    inline VkDevice get_handle() {
        return m_handle;
    };

    inline Swapchain get_swapchain() const {
        return m_swapchain;
    }

    /*
    INIT AND SHUTDOWN
    -----------------------------------------------
    */
    void init( void*           windowHandle,
               WindowingSystem windowingSystem,
               Extent2D        surfaceExtent,
               uint32_t        framesPerFlight,
               ColorFormatType presentFormat,
               SyncType        presentMode );

    void init_headless();

    void update_swapchain( Extent2D surfaceExtent, uint32_t framesPerFlight, ColorFormatType presentFormat, SyncType presentMode );
    void cleanup();

    /*
    OBJECT CREATION
    -----------------------------------------------
    */

    /*Create Buffer using Vulkan Memory Allocator (VMA)*/
    Buffer create_buffer_VMA( size_t allocSize, BufferUsageFlags usage, VmaMemoryUsage memoryUsage, uint32_t strideSize = 0 );
    /*Create Buffer*/
    Buffer create_buffer( size_t allocSize, BufferUsageFlags usage, MemoryPropertyFlags memoryProperties, uint32_t strideSize = 0 );
    /*Create Image*/
    Image create_image( const Extent3D& extent, const ImageConfig& config, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY );
    /*Create Texture*/
    Texture create_texture( const Extent3D& extent, ColorFormatType format, const TextureConfig& config );
    Texture create_texture( Image* img, TextureConfig config );
    /*Create Framebuffer Object*/
    Framebuffer create_framebuffer( const RenderPass& renderpass, const std::vector<Image*>& attachments );
    /*Sync*/
    Semaphore create_semaphore();
    Fence     create_fence( bool signaled = true );
    /*Create RenderPass*/
    RenderPass create_render_pass( const std::vector<RenderTargetInfo>& targets, const std::vector<SubPassDependency>& dependencies );
    /*Create descriptor layout*/
    DescriptorLayout create_descriptor_layout( const std::vector<LayoutBinding>& bindings,
                                               VkDescriptorSetLayoutCreateFlags  flags    = 0,
                                               VkDescriptorBindingFlagsEXT       extFlags = 0 );
    /*Create Descriptor Pool*/
    DescriptorPool create_descriptor_pool( uint32_t maxSets, uint32_t numUBO, uint32_t numUBODynamic, uint32_t numUBOStorage, uint32_t numImageCombined, uint32_t numSampler = 0, uint32_t numSampledImage = 0, uint32_t numStrgImage = 0, uint32_t numUBTexel = 0, uint32_t numStrgTexel = 0, uint32_t numUBOStorageDynamic = 0, uint32_t numIAttachment = 0, VkDescriptorPoolCreateFlagBits flag = {} );
    /*Create Command Pool*/
    CommandPool create_command_pool( QueueType QueueType, CommandPoolCreateFlags flags = COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER );
    /*Create command buffer*/
    CommandBuffer create_command_buffer( CommandPool commandPool, CommandBufferLevel level = COMMAND_BUFFER_LEVEL_PRIMARY );
    /*Create shader pass*/
    GraphicShaderPass create_graphic_shader_pass( const std::string shaderFile, PipelineSettings sett = {} );
    ComputeShaderPass create_compute_shader_pass( const std::string shaderFile, PipelineSettings sett = {} );
    /*
    PRESENTING
    -----------------------------------------------
    */
    RenderResult aquire_present_image( Semaphore& waitSemahpore, uint32_t& imageIndex );
    RenderResult present_image( Semaphore& signalSemaphore, uint32_t imageIndex );
    /*
    DATA TRANSFER
    -----------------------------------------------
    */
    void upload_vertex_arrays( VertexArrays& vao,
                               size_t        vboSize,
                               const void*   vboData,
                               size_t        iboSize,
                               const void*   iboData,
                               size_t        voxelSize = 0,
                               const void*   voxelData = nullptr );
    void upload_texture_image( Texture& tex, const void* imgCache, size_t bytesPerPixel );
    void upload_BLAS( BLAS& accel, VAO& vao );
    void upload_TLAS( TLAS& accel, std::vector<BLASInstance>& BLASinstances );
    void download_texture_image( Texture& tex, void*& imgCache, size_t& size, size_t& channels );
    /*
    MISC
    -----------------------------------------------
    */
    void     wait_idle();
    void     wait_queue_idle( QueueType queueType );
    void     init_imgui( void* windowHandle, WindowingSystem windowingSystem, RenderPass renderPass, uint16_t samples );
    void     destroy_imgui();
    uint32_t get_memory_type( uint32_t typeBits, MemoryPropertyFlags properties, uint32_t* memTypeFound = nullptr );
    /*
    Returns the size of the data having in mind the minimun alginment size per stride in the GPU
    */
    size_t pad_uniform_buffer_size( size_t originalSize );
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif