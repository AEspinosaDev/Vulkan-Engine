#ifndef VK_SWAPCHAIN
#define VK_SWAPCHAIN

#include "vk_core.h"
#include "vk_bootstrap.h"
#include "vk_initializers.h"
#include "vk_image.h"

namespace vke
{

	class Swapchain
	{
	private:
		VkSwapchainKHR m_swapchain;

		VkFormat m_presentFormat;
		std::vector<VkImage> m_presentImages;
		std::vector<VkImageView> m_presentImageViews;

		//Resources
		Image m_colorBuffer{};
		Image m_depthBuffer{};

		std::vector<VkFramebuffer> m_framebuffers;

		VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
		void create_image_views(VkDevice &device);

	public:
		void create(VkPhysicalDevice &gpu, VkDevice &device, VkSurfaceKHR &surface, GLFWwindow *window, VkExtent2D &windowExtent);
		void create_colorbuffer(VkDevice &device, VmaAllocator &memory, VkExtent2D &windowExtent, VkSampleCountFlagBits samples);
		void create_depthbuffer(VkDevice &device, VmaAllocator &memory, VkExtent2D &windowExtent, VkSampleCountFlagBits samples);
		void create_framebuffers(VkDevice &device, VkRenderPass &defaultRenderPass, VkExtent2D &windowExtent, VkSampleCountFlagBits samples);
		void cleanup(VkDevice &device, VmaAllocator &memory);

		inline VkSwapchainKHR& get_swapchain_obj()
		{
			return m_swapchain;
		}
		inline VkFormat& get_image_format()
		{
			return m_presentFormat;
		}
		inline std::vector<VkImage> get_images()
		{
			return m_presentImages;
		}
		inline std::vector<VkImageView> get_image_views()
		{
			return m_presentImageViews;
		}
		inline std::vector<VkFramebuffer> get_framebuffers()
		{
			return m_framebuffers;
		}
		inline Image& get_colorbuffer()
		{
			return m_colorBuffer;
		}
		inline Image& get_depthbuffer()
		{
			return m_depthBuffer;
		}
	};

}
#endif