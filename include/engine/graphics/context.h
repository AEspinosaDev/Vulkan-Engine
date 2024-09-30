/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef CONTEXT_H
#define CONTEXT_H

// #include <functional>

#include <engine/common.h>

#include <engine/graphics/utils.h>
#include <engine/graphics/bootstrap.h>
#include <engine/graphics/initializers.h>
#include <engine/graphics/swapchain.h>
#include <engine/graphics/frame.h>


VULKAN_ENGINE_NAMESPACE_BEGIN

struct Context
{
    Swapchain swapchain;

    VkSurfaceKHR surface{};

    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice gpu{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VmaAllocator memory{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};

    VkQueue graphicsQueue{};
    VkQueue presentQueue{};

    utils::UploadContext uploadContext{};

#ifdef NDEBUG
    const bool enableValidationLayers{false};
#else
    const bool enableValidationLayers{true};
#endif

    void init(GLFWwindow *windowHandle, VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode);

    void recreate_swapchain(GLFWwindow *windowHandle, VkExtent2D surfaceExtent, uint32_t framesPerFlight, VkFormat presentFormat, VkPresentModeKHR presentMode);

    void cleanup();

    void wait_for_device();

    uint32_t aquire_present_image(Frame &currentFrame);

    void begin_command_buffer(Frame &currentFrame);

    void end_command_buffer(Frame &currentFrame);

    VkResult present_image(Frame &currentFrame, uint32_t imageIndex);

};

VULKAN_ENGINE_NAMESPACE_END

#endif