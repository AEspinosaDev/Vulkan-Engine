#include <engine/backend/vk_utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

bool utils::check_validation_layer_suport(std::vector<const char *> validationLayers)
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto &layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

VkPhysicalDeviceFeatures utils::get_gpu_features(VkPhysicalDevice &gpu)
{

	VkPhysicalDeviceFeatures deviceFeatures{};
	vkGetPhysicalDeviceFeatures(gpu, &deviceFeatures);
	return deviceFeatures;
}

VkPhysicalDeviceProperties utils::get_gpu_properties(VkPhysicalDevice &gpu)
{
	VkPhysicalDeviceProperties deviceFeatures;
	vkGetPhysicalDeviceProperties(gpu, &deviceFeatures);
	return deviceFeatures;
}
size_t utils::pad_uniform_buffer_size(size_t originalSize, VkPhysicalDevice &gpu)
{
	VkPhysicalDeviceProperties deviceFeatures;
	vkGetPhysicalDeviceProperties(gpu, &deviceFeatures);
	// Calculate required alignment based on minimum device offset alignment
	size_t minUboAlignment = deviceFeatures.limits.minUniformBufferOffsetAlignment;
	size_t alignedSize = originalSize;
	if (minUboAlignment > 0)
	{
		alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return alignedSize;
}
uint32_t utils::find_memory_type(VkPhysicalDevice &gpu, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}
void utils::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void utils::destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}
void utils::log_available_extensions(std::vector<VkExtensionProperties> ext)
{
	DEBUG_LOG("---------------------");
	DEBUG_LOG("Available extensions");
	DEBUG_LOG("---------------------");
	for (const auto &extension : ext)
	{
		DEBUG_LOG(extension.extensionName);
	}
	DEBUG_LOG("---------------------");
}
void utils::log_available_gpus(std::multimap<int, VkPhysicalDevice> candidates)
{
	DEBUG_LOG("---------------------");
	DEBUG_LOG("Suitable Devices");
	DEBUG_LOG("---------------------");
	for (const auto &candidate : candidates)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(candidate.second, &deviceProperties);
		DEBUG_LOG(deviceProperties.deviceName);
	}
	DEBUG_LOG("---------------------");
}
VkResult utils::create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
Vec3 utils::get_tangent_gram_smidt(Vec3 &p1, Vec3 &p2, Vec3 &p3, glm::vec2 &uv1, glm::vec2 &uv2, glm::vec2 &uv3, Vec3 normal)
{

	Vec3 edge1 = p2 - p1;
	Vec3 edge2 = p3 - p1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	Vec3 tangent;
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

	// Gram-Schmidt orthogonalization
	return glm::normalize(tangent - normal * glm::dot(normal, tangent));

	// return glm::normalize(tangent);
}

VULKAN_ENGINE_NAMESPACE_END