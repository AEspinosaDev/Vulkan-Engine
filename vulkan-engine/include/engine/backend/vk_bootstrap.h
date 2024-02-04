#ifndef VK_BOOTSTRAP
#define VK_BOOTSTRAP

#include <engine/vk_common.h>
#include "vk_utils.h"

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace vkboot
{

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		inline bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanBooter
	{
	public:
		VulkanBooter(VkInstance *_instance,
					 VkDebugUtilsMessengerEXT *_debugMessenger,
					 VkPhysicalDevice *_gpu,
					 VkDevice *_device,
					 VkQueue *_graphicsQueue, VkSurfaceKHR *_surface,
					 VkQueue *_presentQueue, VmaAllocator *mem, bool validate) : m_instance(_instance),
																				 m_debugMessenger(_debugMessenger), m_gpu(_gpu),
																				 m_device(_device), m_graphicsQueue(_graphicsQueue),
																				 m_presentQueue(_presentQueue), m_surface(_surface),
																				 m_validation(validate), m_memory(mem)
		{
		}
		void boot_vulkan();
		void setup_devices();
		void pick_graphics_card_device();
		void create_logical_device(VkPhysicalDeviceFeatures features);
		void setup_memory();

	private:
		VkInstance *m_instance;
		VkDebugUtilsMessengerEXT *m_debugMessenger;
		VkPhysicalDevice *m_gpu;
		VkDevice *m_device;
		VkSurfaceKHR *m_surface;
		VkQueue *m_graphicsQueue;
		VkQueue *m_presentQueue;
		VmaAllocator *m_memory;
		bool m_validation;

		const std::vector<const char *> m_validationLayers = {
			"VK_LAYER_KHRONOS_validation"};
		std::vector<const char *> m_deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		void create_instance();
		std::vector<const char *> get_required_extensions();
		void setup_debug_messenger();
		int rate_device_suitability(VkPhysicalDevice device);
		bool check_device_extension_support(VkPhysicalDevice device);
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
	SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface);

}

VULKAN_ENGINE_NAMESPACE_END
#endif