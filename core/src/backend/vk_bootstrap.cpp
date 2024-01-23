#define VMA_IMPLEMENTATION
#include "vk_bootstrap.h"

namespace vke
{

	void vkboot::VulkanBooter::create_instance()
	{
		if (m_validation && !vkutils::check_validation_layer_suport(m_validationLayers))
		{
			throw std::runtime_error(" validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "UserDeclare";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "VK Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3; // To enable the most extensions

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = get_required_extensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		if (m_validation)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();

			vkutils::populate_debug_messenger_create_info(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, m_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}

	std::vector<const char *> vkboot::VulkanBooter::get_required_extensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_validation)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
#ifndef NDEBUG
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> supported_extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supported_extensions.data());
		vkutils::log_available_extensions(supported_extensions);
#endif
		return extensions;
	}

	void vkboot::VulkanBooter::setup_debug_messenger()
	{
		if (!m_validation)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		vkutils::populate_debug_messenger_create_info(createInfo);

		if (vkutils::create_debug_utils_messenger_EXT(*m_instance, &createInfo, nullptr, m_debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void vkboot::VulkanBooter::pick_graphics_card_device()
	{
		*m_gpu = VK_NULL_HANDLE;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(*m_instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(*m_instance, &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto &device : devices)
		{
			int score = rate_device_suitability(device);

			candidates.insert(std::make_pair(score, device));
		}
#ifndef NDEBUG
		vkutils::log_available_gpus(candidates);
#endif // !NDEBUG

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0)
		{
			*m_gpu = candidates.rbegin()->second;
		}
		else
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}
	void vkboot::VulkanBooter::create_logical_device(VkPhysicalDeviceFeatures features)
	{
		vkboot::QueueFamilyIndices indices = vkboot::find_queue_families(*m_gpu, *m_surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};

		if (vkutils::is_device_extension_supported(*m_gpu, "VK_EXT_extended_dynamic_state"))
		{
			m_deviceExtensions.push_back("VK_EXT_extended_dynamic_state");

			// Request extended features
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

			// Add extension-specific structures here
			VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
			extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
			extendedDynamicStateFeatures.pNext = physicalDeviceFeatures2.pNext; // Chain with existing features

			physicalDeviceFeatures2.pNext = &extendedDynamicStateFeatures;

			vkGetPhysicalDeviceFeatures2(*m_gpu, &physicalDeviceFeatures2);
		}
		physicalDeviceFeatures2.features = features;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pNext = &physicalDeviceFeatures2;

		// createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

		if (m_validation)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}
		if (vkCreateDevice(*m_gpu, &createInfo, nullptr, m_device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}
		vkGetDeviceQueue(*m_device, indices.graphicsFamily.value(), 0, m_graphicsQueue);
		vkGetDeviceQueue(*m_device, indices.presentFamily.value(), 0, m_presentQueue);
	}

	int vkboot::VulkanBooter::rate_device_suitability(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;

		// VR, 64B floats and multi-viewport
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		vkboot::QueueFamilyIndices indices = find_queue_families(device, *m_surface);

		// Application can't function without geometry shaders
		if (!deviceFeatures.geometryShader || !indices.isComplete())
		{
			return 0;
		}
		bool swapChainAdequate = false;
		if (check_device_extension_support(device))
		{
			vkboot::SwapChainSupportDetails swapChainSupport = query_swapchain_support(device, *m_surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			if (!swapChainAdequate)
				return 0;
		}
		else
		{
			return 0;
		}

		return score;
	}

	bool vkboot::VulkanBooter::check_device_extension_support(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	vkboot::QueueFamilyIndices vkboot::find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		vkboot::QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}
			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	vkboot::SwapChainSupportDetails vkboot::query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		vkboot::SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		return details;
	}

	void vkboot::VulkanBooter::boot_vulkan()
	{
		create_instance();
		setup_debug_messenger();
	}

	void vkboot::VulkanBooter::setup_devices( )
	{

		pick_graphics_card_device();

		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceFeatures(*m_gpu, &deviceFeatures);
		create_logical_device(deviceFeatures);
	}

	void vkboot::VulkanBooter::setup_memory()
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = *m_gpu;
		allocatorInfo.device = *m_device;
		allocatorInfo.instance = *m_instance;
		vmaCreateAllocator(&allocatorInfo, m_memory);
	}
}