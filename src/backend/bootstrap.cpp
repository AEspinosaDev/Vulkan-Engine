#define VMA_IMPLEMENTATION
#include <engine/backend/bootstrap.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

VkInstance boot::VulkanBooter::create_instance()
{
	if (m_validation && !utils::check_validation_layer_suport(m_validationLayers))
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

		utils::populate_debug_messenger_create_info(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	VkInstance instance{};
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
	return instance;
}

std::vector<const char *> boot::VulkanBooter::get_required_extensions()
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
	utils::log_available_extensions(supported_extensions);
#endif
	return extensions;
}

VkPhysicalDevice boot::VulkanBooter::pick_graphics_card_device(VkInstance instance, VkSurfaceKHR surface)
{
	VkPhysicalDevice gpu = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto &device : devices)
	{
		int score = rate_device_suitability(device, surface);

		candidates.insert(std::make_pair(score, device));
	}
#ifndef NDEBUG
	utils::log_available_gpus(candidates);
#endif // !NDEBUG

	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0)
	{
		gpu = candidates.rbegin()->second;
	}
	else
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	return gpu;
}
VkDevice boot::VulkanBooter::create_logical_device(
	VkQueue &graphicsQueue,
	VkQueue &presentQueue,
	VkPhysicalDevice gpu,
	VkPhysicalDeviceFeatures features,
	VkSurfaceKHR surface)
{
	boot::QueueFamilyIndices indices = boot::find_queue_families(gpu, surface);

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

	if (utils::is_device_extension_supported(gpu, "VK_EXT_extended_dynamic_state"))
	{
		m_deviceExtensions.push_back("VK_EXT_extended_dynamic_state");

		// Request extended features
		physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

		// Add extension-specific structures here
		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
		extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
		extendedDynamicStateFeatures.pNext = physicalDeviceFeatures2.pNext; // Chain with existing features

		physicalDeviceFeatures2.pNext = &extendedDynamicStateFeatures;

		vkGetPhysicalDeviceFeatures2(gpu, &physicalDeviceFeatures2);
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
	VkDevice device{};
	if (vkCreateDevice(gpu, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	return device;
}

int boot::VulkanBooter::rate_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface)
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

	boot::QueueFamilyIndices indices = find_queue_families(device, surface);

	// Application can't function without geometry shaders
	if (!deviceFeatures.geometryShader || !indices.isComplete())
	{
		return 0;
	}
	bool swapChainAdequate = false;
	if (check_device_extension_support(device))
	{
		boot::SwapChainSupportDetails swapChainSupport = query_swapchain_support(device, surface);
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

bool boot::VulkanBooter::check_device_extension_support(VkPhysicalDevice device)
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

boot::QueueFamilyIndices boot::find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	boot::QueueFamilyIndices indices;

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

boot::SwapChainSupportDetails boot::query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	boot::SwapChainSupportDetails details;
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

VkInstance boot::VulkanBooter::boot_vulkan()
{
	VkInstance instance = create_instance();
	return instance;
}

VkDebugUtilsMessengerEXT boot::VulkanBooter::create_debug_messenger(VkInstance instance)
{
	if (!m_validation)
		return VK_NULL_HANDLE;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	utils::populate_debug_messenger_create_info(createInfo);

	VkDebugUtilsMessengerEXT debugMessenger{};
	if (utils::create_debug_utils_messenger_EXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
	return debugMessenger;
}
VmaAllocator boot::VulkanBooter::setup_memory(VkInstance instance, VkDevice device, VkPhysicalDevice gpu)
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = gpu;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	VmaAllocator memoryAllocator;
	vmaCreateAllocator(&allocatorInfo, &memoryAllocator);
	return memoryAllocator;
}
VULKAN_ENGINE_NAMESPACE_END