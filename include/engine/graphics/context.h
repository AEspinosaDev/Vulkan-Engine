/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CONTEXT_H
#define CONTEXT_H

// #include <functional>

#include <engine/common.h>

#include <engine/graphics/bootstrap.h>
#include <engine/graphics/extensions.h>
#include <engine/graphics/frame.h>
#include <engine/graphics/initializers.h>
#include <engine/graphics/swapchain.h>
#include <engine/graphics/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{
/*
Vulkan API graphic context related data abnd functionality
*/
struct Context
{
    Swapchain swapchain{};

    VkSurfaceKHR surface{};

    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice gpu{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VmaAllocator memory{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};

    VkQueue graphicsQueue{};
    VkQueue presentQueue{};

    VkDescriptorPool m_guiPool{};

    std::vector<Frame> frames;

    utils::UploadContext uploadContext{};

#ifdef NDEBUG
    const bool enableValidationLayers{false};
#else
    const bool enableValidationLayers{true};
#endif

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
    void upload_vertex_arrays(Buffer &vbo, size_t vboSize, const void *vboData, Buffer &ibo, size_t iboSize,
                              const void *iboData, bool indexed);

    void upload_texture_image(const void *imgCache, size_t bytesPerPixel, Image *const img, bool mipmapping);

    /*
    MISC
    -----------------------------------------------
    */
    void wait_for_device();

    void init_gui_pool();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif