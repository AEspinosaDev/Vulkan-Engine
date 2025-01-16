#define VMA_IMPLEMENTATION
#include <engine/graphics/utilities/bootstrap.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
#pragma region INSTANCE
VkInstance Booter::create_instance(const char*              appName,
                                   const char*              engineName,
                                   bool                     validation,
                                   std::vector<const char*> validationLayers) {
    if (validation && !Utils::check_validation_layer_suport(validationLayers))
    {
        throw VKFW_Exception(" validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = appName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = engineName;
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_3; // To enable the most extensions

    VkInstanceCreateInfo createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions                    = get_required_extensions(validation);
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if (validation)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        Utils::populate_debug_messenger_create_info(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    VkInstance instance{};
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw VKFW_Exception("failed to create instance!");
    }
    return instance;
}

std::vector<const char*> Booter::get_required_extensions(bool validation) {
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (validation)
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
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    return extensions;
}
#pragma region GPU

VkPhysicalDevice
Booter::pick_graphics_card_device(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*> extensions) {
    VkPhysicalDevice gpu = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw VKFW_Exception("failed to find GPUs with Vulkan support!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices)
    {
        int score = rate_device_suitability(device, surface, extensions);

        candidates.insert(std::make_pair(score, device));
    }
#ifndef NDEBUG
    Utils::log_available_gpus(candidates);
#endif // !NDEBUG

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0)
    {
        gpu = candidates.rbegin()->second;
    } else
    {
        throw VKFW_Exception("failed to find a suitable GPU!");
    }

    return gpu;
}

int Booter::rate_device_suitability(VkPhysicalDevice         device,
                                    VkSurfaceKHR             surface,
                                    std::vector<const char*> extensions) {
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

    Utils::QueueFamilyIndices indices = Utils::find_queue_families(device, surface);

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader || !indices.isComplete())
    {
        return 0;
    }
    bool swapChainAdequate = false;
    if (check_device_extension_support(device, extensions))
    {
        Utils::SwapChainSupportDetails swapChainSupport = Utils::query_swapchain_support(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        if (!swapChainAdequate)
            return 0;
    } else
    {
        return 0;
    }

    return score;
}

bool Booter::check_device_extension_support(VkPhysicalDevice device, std::vector<const char*> extensions) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

#pragma region DEVICE
VkDevice Booter::create_logical_device(std::unordered_map<QueueType, VkQueue>& queues,
                                       VkPhysicalDevice                        gpu,
                                       VkPhysicalDeviceFeatures                features,
                                       VkSurfaceKHR                            surface,
                                       bool                                    validation,
                                       std::vector<const char*>                validationLayers) {

    Utils::QueueFamilyIndices            queueFamilies = Utils::find_queue_families(gpu, surface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t>                   uniqueQueueFamilies = {
        queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value(), queueFamilies.computeFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    std::vector<const char*> enabledExtensions;

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.pNext                     = nullptr;

    enabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    if (Utils::is_device_extension_supported(gpu, "VK_NV_geometry_shader_passthrough"))
        enabledExtensions.push_back("VK_NV_geometry_shader_passthrough");

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};

    if (Utils::is_device_extension_supported(gpu, "VK_EXT_extended_dynamic_state"))
    {
        enabledExtensions.push_back("VK_EXT_extended_dynamic_state");

        extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
        extendedDynamicStateFeatures.pNext = physicalDeviceFeatures2.pNext;

        physicalDeviceFeatures2.pNext = &extendedDynamicStateFeatures;
    }

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

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR    rayTracingPipelineFeatures    = {};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR   bufferDeviceAddressFeatures   = {};
    VkPhysicalDeviceDescriptorIndexingFeaturesEXT    descriptorIndexingFeatures    = {};
    VkPhysicalDeviceRayQueryFeaturesKHR              rayQueryFeatures              = {};

    // Check RTX extensions
    if (Utils::is_device_extension_supported(gpu, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) &&
        Utils::is_device_extension_supported(gpu, VK_KHR_RAY_QUERY_EXTENSION_NAME))
    {

        enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        enabledExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

        rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        rayTracingPipelineFeatures.pNext = physicalDeviceFeatures2.pNext;

        accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelerationStructureFeatures.pNext = &rayTracingPipelineFeatures;
        accelerationStructureFeatures.accelerationStructure = true;

        rayQueryFeatures.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
        rayQueryFeatures.pNext    = &accelerationStructureFeatures;
        rayQueryFeatures.rayQuery = true;

        bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
        bufferDeviceAddressFeatures.pNext = &rayQueryFeatures;
        bufferDeviceAddressFeatures.bufferDeviceAddress = true;

        descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        descriptorIndexingFeatures.pNext = &bufferDeviceAddressFeatures;

        // Finally, attach the descriptorIndexingFeatures to the physicalDeviceFeatures2
        physicalDeviceFeatures2.pNext = &descriptorIndexingFeatures;
    }

    physicalDeviceFeatures2.features = features;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    createInfo.pNext                = &physicalDeviceFeatures2;

    // createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    if (validation)
    {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else
    {
        createInfo.enabledLayerCount = 0;
    }
    VkDevice device{};
    if (vkCreateDevice(gpu, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, queueFamilies.graphicsFamily.value(), 0, &queues[QueueType::GRAPHIC_QUEUE]);
    vkGetDeviceQueue(device, queueFamilies.presentFamily.value(), 0, &queues[QueueType::PRESENT_QUEUE]);
    vkGetDeviceQueue(device, queueFamilies.computeFamily.value(), 0, &queues[QueueType::COMPUTE_QUEUE]);

    return device;
}

#pragma region DEBUG

VkDebugUtilsMessengerEXT Booter::create_debug_messenger(VkInstance instance) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    Utils::populate_debug_messenger_create_info(createInfo);

    VkDebugUtilsMessengerEXT debugMessenger{};
    if (Utils::create_debug_utils_messenger_EXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw VKFW_Exception("failed to set up debug messenger!");
    }
    return debugMessenger;
}
#pragma region VMA
VmaAllocator Booter::setup_memory(VkInstance instance, VkDevice device, VkPhysicalDevice gpu) {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice         = gpu;
    allocatorInfo.device                 = device;
    allocatorInfo.instance               = instance;
    allocatorInfo.flags                  = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    VmaAllocator memoryAllocator;
    vmaCreateAllocator(&allocatorInfo, &memoryAllocator);
    return memoryAllocator;
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END