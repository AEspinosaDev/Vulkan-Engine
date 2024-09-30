/*
	This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

	MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <engine/common.h>
#include <engine/graphics/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace boot
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
		VulkanBooter(bool validate)
			: m_validation(validate)
		{
		}

		VkInstance boot_vulkan();

		VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance);

		VkPhysicalDevice pick_graphics_card_device(
			VkInstance instance,
			VkSurfaceKHR surface);

		VkDevice create_logical_device(
			VkQueue &graphicsQueue,
			VkQueue &presentQueue,
			VkPhysicalDevice gpu,
			VkPhysicalDeviceFeatures features,
			VkSurfaceKHR surface);

		VmaAllocator setup_memory(
			VkInstance instance,
			VkDevice device,
			VkPhysicalDevice gpu);

	private:
		bool m_validation;

		const std::vector<const char *> m_validationLayers = {
			"VK_LAYER_KHRONOS_validation"};
		std::vector<const char *> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
														"VK_EXT_extended_dynamic_state",
														VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
														VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
														VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
														VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

		VkInstance create_instance();
		std::vector<const char *> get_required_extensions();
		int rate_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface);
		bool check_device_extension_support(VkPhysicalDevice device);
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
	SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface);

}

VULKAN_ENGINE_NAMESPACE_END
#endif