/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VK_SWAPCHAIN
#define VK_SWAPCHAIN

#include <engine/vk_common.h>
#include "vk_bootstrap.h"
#include "vk_initializers.h"
#include "vk_image.h"

VULKAN_ENGINE_NAMESPACE_BEGIN

class Swapchain
{
private:
	VkSwapchainKHR m_swapchain;

	VkFormat m_presentFormat;
	VkPresentModeKHR m_presentMode;

	std::vector<Image> m_presentImages;

	std::vector<VkFramebuffer> m_framebuffers;

	VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats, VkFormat desiredFormat);
	VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &availablePresentModes, VkPresentModeKHR desiredMode);
	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
	void create_image_views(VkDevice &device);
	

public:
	void create(VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface,
				GLFWwindow *window, VkExtent2D &windowExtent, uint32_t imageCount = 2 ,VkFormat userDefinedcolorFormat = VK_FORMAT_B8G8R8A8_SRGB, VkPresentModeKHR userDefinedPresentMode = VK_PRESENT_MODE_MAILBOX_KHR);
	void cleanup(VkDevice &device, VmaAllocator &memory);

	inline VkSwapchainKHR &get_swapchain_obj()
	{
		return m_swapchain;
	}
	inline VkFormat &get_image_format()
	{
		return m_presentFormat;
	}
	inline std::vector<Image> get_present_images()
	{
		return m_presentImages;
	}
	
};

VULKAN_ENGINE_NAMESPACE_END
#endif