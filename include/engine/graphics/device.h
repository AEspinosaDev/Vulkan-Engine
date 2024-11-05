/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CONTEXT_H
#define CONTEXT_H

#include <engine/common.h>

#include <engine/graphics/extensions.h>
#include <engine/graphics/frame.h>
#include <engine/graphics/swapchain.h>
#include <engine/graphics/command_buffer.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/utilities/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{
/*
Vulkan API graphic context related data and functionality
*/
struct Device
{
    VkDevice handle{VK_NULL_HANDLE};

    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice gpu{VK_NULL_HANDLE};
    VmaAllocator memory{VK_NULL_HANDLE};

    Swapchain swapchain{};
    std::vector<Frame> frames;

    Utils::QueueFamilyIndices queueFamilies{};
    std::unordered_map<QueueType, VkQueue> queues;

    VkDescriptorPool m_guiPool{};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};

    Utils::UploadContext uploadContext{};

#ifdef NDEBUG
    const bool enableValidationLayers{false};
#else
    const bool enableValidationLayers{true};
#endif
    const std::vector<const char *> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char *> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                    "VK_EXT_extended_dynamic_state",
                                                    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                                    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                                                    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                                    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

    /*
    INIT AND SHUTDOWN
    -----------------------------------------------
    */

    void init(void *windowHandle, WindowingSystem windowingSystem, VkExtent2D surfaceExtent, uint32_t framesPerFlight,
              VkFormat presentFormat, VkPresentModeKHR presentMode);

    void update_swapchain(VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat,
                          VkPresentModeKHR presentMode);

    void cleanup();

    /*
    OBJECT INITIALIZAITON
    -----------------------------------------------
    */

    void init_buffer(Buffer &buffer, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                       uint32_t istrideSize, std::vector<uint32_t> stridePartitionsSizes = {});

    void init_image(Image &image, bool useMipmaps, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);

    void init_command_pool(CommandPool &pool, QueueType type);

    // void create_descriptor_manager();

    // void create_render_pass();

    // void create_graphic_pipeline();

    // void create_compute_pipeline();

    // void create_framebuffer();

    /*
    DRAWING
    -----------------------------------------------
    */

    VkResult aquire_present_image(const uint32_t &currentFrame, uint32_t &imageIndex);

    void begin_command_buffer(const uint32_t &currentFrame);

    void end_command_buffer(const uint32_t &currentFrame);

    VkResult present_image(const uint32_t &currentFrame, uint32_t imageIndex);

    static void draw_geometry(VkCommandBuffer &cmd, Buffer &vbo, Buffer &ibo, uint32_t vertexCount, uint32_t indexCount,
                              bool indexed, uint32_t instanceCount = 1, uint32_t firstOcurrence = 0, int32_t offset = 0,
                              uint32_t firstInstance = 0);

    static void draw_empty(VkCommandBuffer &cmd, uint32_t vertexCount = 0, uint32_t instanceCount = 1);

    /*
    DATA TRANSFER
    -----------------------------------------------
    */
    void upload_vertex_arrays(VertexArrays &vao, size_t vboSize, const void *vboData, size_t iboSize,
                              const void *iboData);

    void destroy_vertex_arrays(VertexArrays &vao);

    void upload_texture_image(const void *imgCache, size_t bytesPerPixel, Image *const img, bool mipmapping);

    void destroy_texture_image(Image *const img);

    /*
    MISC
    -----------------------------------------------
    */
    void wait();

    void init_gui_pool();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif