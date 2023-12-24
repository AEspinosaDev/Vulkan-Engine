#ifndef VK_SWAPCHAIN
#define VK_SWAPCHAIN

#include "vk_core.h"
#include "vk_bootstrap.h"

namespace vke
{

	class Swapchain
	{
	private:
		VkSwapchainKHR m_swapchain;
		VkFormat m_swapchainImageFormat;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;

		VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
		void create_image_views(VkDevice *device);

	public:
		void create(VkPhysicalDevice *gpu, VkDevice *device, VkSurfaceKHR *surface, GLFWwindow *window, VkExtent2D *windowExtent);
		void cleanup(VkDevice *device);

		inline VkSwapchainKHR *get_swapchain_obj()
		{
			return &m_swapchain;
		}
		inline VkFormat *get_image_format()
		{
			return &m_swapchainImageFormat;
		}
		inline std::vector<VkImage> get_images()
		{
			return m_swapchainImages;
		}
		inline std::vector<VkImageView> get_image_views()
		{
			return m_swapchainImageViews;
		}
	};

}
#endif