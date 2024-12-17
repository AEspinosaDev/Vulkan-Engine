/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <engine/common.h>
#include <engine/graphics/image.h>
#include <engine/graphics/utilities/bootstrap.h>
#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
class Swapchain
{
    VkSwapchainKHR             m_handle;
    VkDevice                   m_device;
    VkSurfaceKHR               m_surface;
    VkFormat                   m_presentFormat;
    VkPresentModeKHR           m_presentMode;
    std::vector<Image>         m_presentImages;

    bool m_initialized{false};

    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats,
                                                  VkFormat                               desiredFormat);
    VkPresentModeKHR   choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes,
                                                VkPresentModeKHR                     desiredMode);
    VkExtent2D         choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D actualExtent);

    void create_image_views(VkDevice& device);

  public:
    VkExtent2D create_surface(VkInstance instance, void* windowHandle, WindowingSystem windowingSystem);
    void       destroy_surface(VkInstance instance);

    void create(VkPhysicalDevice& gpu,
                VkDevice&         device,
                VkExtent2D        actualExtent,
                VkExtent2D        windowExtent,
                uint32_t          imageCount             = 2,
                VkFormat          userDefinedcolorFormat = VK_FORMAT_B8G8R8A8_SRGB,
                VkPresentModeKHR  userDefinedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR);
    void cleanup();

    inline VkSwapchainKHR& get_handle() {
        return m_handle;
    }
    inline VkSurfaceKHR& get_surface() {
        return m_surface;
    }
    inline VkFormat& get_image_format() {
        return m_presentFormat;
    }
    inline std::vector<Image>& get_present_images() {
        return m_presentImages;
    }
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif