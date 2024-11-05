#define VMA_IMPLEMENTATION
#include <engine/graphics/utilities/bootstrap.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

VkInstance VKBooter::create_instance()
{
    if (m_validation && !Utils::check_validation_layer_suport(m_validationLayers))
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

        Utils::populate_debug_messenger_create_info(debugCreateInfo);
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

std::vector<const char *> VKBooter::get_required_extensions()
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
    Utils::log_available_extensions(supported_extensions);
#endif
    return extensions;
}

VkPhysicalDevice VKBooter::pick_graphics_card_device(VkInstance instance, VkSurfaceKHR surface)
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
    Utils::log_available_gpus(candidates);
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
VkDevice VKBooter::create_logical_device(VkQueue &graphicsQueue, VkQueue &presentQueue, VkPhysicalDevice gpu,
                                         VkPhysicalDeviceFeatures features, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices = find_queue_families(gpu, surface);

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

    std::vector<const char *> enabledExtensions;

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext = nullptr;

    enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};

    if (Utils::is_device_extension_supported(gpu, "VK_EXT_extended_dynamic_state"))
    {
        enabledExtensions.push_back("VK_EXT_extended_dynamic_state");

        extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
        extendedDynamicStateFeatures.pNext = physicalDeviceFeatures2.pNext;

        physicalDeviceFeatures2.pNext = &extendedDynamicStateFeatures;
    }

    // if (utils::is_device_extension_supported(gpu, "VK_EXT_swapchain_colorspace"))
    // {
    //     enabledExtensions.push_back("VK_EXT_swapchain_colorspace");

    //     // Link the extended dynamic state features to the physical device features
    //     extendedDynamicStateFeatures.pNext = physicalDeviceFeatures2.pNext;
    //     physicalDeviceFeatures2.pNext = &extendedDynamicStateFeatures;
    // }

    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = {};

    if (Utils::is_device_extension_supported(gpu, "VK_EXT_extended_dynamic_state2"))
    {
        enabledExtensions.push_back("VK_EXT_extended_dynamic_state2");

        extendedDynamicState2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
        extendedDynamicState2Features.pNext = physicalDeviceFeatures2.pNext;

        physicalDeviceFeatures2.pNext = &extendedDynamicState2Features;
    }

    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3Features = {};

    if (Utils::is_device_extension_supported(gpu, "VK_EXT_extended_dynamic_state3"))
    {
        enabledExtensions.push_back("VK_EXT_extended_dynamic_state3");

        extendedDynamicState3Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
        extendedDynamicState3Features.pNext = physicalDeviceFeatures2.pNext;
        extendedDynamicState3Features.extendedDynamicState3PolygonMode = VK_TRUE;

        physicalDeviceFeatures2.pNext = &extendedDynamicState3Features;
    }

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures = {};
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures = {};

    // Check RTX extensions
    if (Utils::is_device_extension_supported(gpu, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
    {

        enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        enabledExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

        rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        rayTracingPipelineFeatures.pNext = nullptr;

        accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;

        bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
        bufferDeviceAddressFeatures.pNext = &accelerationStructureFeatures;

        descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        descriptorIndexingFeatures.pNext = &bufferDeviceAddressFeatures;

        // Finally, attach the descriptorIndexingFeatures to the physicalDeviceFeatures2
        descriptorIndexingFeatures.pNext = physicalDeviceFeatures2.pNext;
        physicalDeviceFeatures2.pNext = &descriptorIndexingFeatures;
    }

    physicalDeviceFeatures2.features = features;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pNext = &physicalDeviceFeatures2;

    // createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

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

int VKBooter::rate_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface)
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

    QueueFamilyIndices indices = find_queue_families(device, surface);

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader || !indices.isComplete())
    {
        return 0;
    }
    bool swapChainAdequate = false;
    if (check_device_extension_support(device))
    {
        SwapChainSupportDetails swapChainSupport = query_swapchain_support(device, surface);
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

bool VKBooter::check_device_extension_support(VkPhysicalDevice device)
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

QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

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

SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;
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

VkInstance VKBooter::boot_vulkan()
{
    VkInstance instance = create_instance();
    return instance;
}

VkDebugUtilsMessengerEXT VKBooter::create_debug_messenger(VkInstance instance)
{
    if (!m_validation)
        return VK_NULL_HANDLE;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    Utils::populate_debug_messenger_create_info(createInfo);

    VkDebugUtilsMessengerEXT debugMessenger{};
    if (Utils::create_debug_utils_messenger_EXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
    return debugMessenger;
}
VmaAllocator VKBooter::setup_memory(VkInstance instance, VkDevice device, VkPhysicalDevice gpu)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = gpu;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    VmaAllocator memoryAllocator;
    vmaCreateAllocator(&allocatorInfo, &memoryAllocator);
    return memoryAllocator;
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END