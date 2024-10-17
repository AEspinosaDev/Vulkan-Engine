#include <engine/graphics/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace graphics
{

std::string utils::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}
std::string utils::read_file(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open #include script: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

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
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void utils::destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks *pAllocator)
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
VkResult utils::create_debug_utils_messenger_EXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator,
                                                 VkDebugUtilsMessengerEXT *pDebugMessenger)
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
Vec3 utils::get_tangent_gram_smidt(Vec3 &p1, Vec3 &p2, Vec3 &p3, glm::vec2 &uv1, glm::vec2 &uv2, glm::vec2 &uv3,
                                   Vec3 normal)
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
void utils::compute_tangents_gram_smidt(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
    if (!indices.empty())
        for (size_t i = 0; i < indices.size(); i += 3)
        {
            size_t i0 = indices[i];
            size_t i1 = indices[i + 1];
            size_t i2 = indices[i + 2];

            Vec3 tangent =
                get_tangent_gram_smidt(vertices[i0].pos, vertices[i1].pos, vertices[i2].pos, vertices[i0].texCoord,
                                       vertices[i1].texCoord, vertices[i2].texCoord, vertices[i0].normal);

            vertices[i0].tangent += tangent;
            vertices[i1].tangent += tangent;
            vertices[i2].tangent += tangent;
        }
    else
        for (size_t i = 0; i < vertices.size(); i += 3)
        {
            Vec3 tangent =
                get_tangent_gram_smidt(vertices[i].pos, vertices[i + 1].pos, vertices[i + 2].pos, vertices[i].texCoord,
                                       vertices[i + 1].texCoord, vertices[i + 2].texCoord, vertices[i].normal);

            vertices[i].tangent += tangent;
            vertices[i + 1].tangent += tangent;
            vertices[i + 2].tangent += tangent;
        }
}

void utils::UploadContext::init(VkDevice &device, VkPhysicalDevice &gpu, VkSurfaceKHR surface)
{
    VkFenceCreateInfo uploadFenceCreateInfo = init::fence_create_info();

    VK_CHECK(vkCreateFence(device, &uploadFenceCreateInfo, nullptr, &uploadFence));

    VkCommandPoolCreateInfo uploadCommandPoolInfo =
        init::command_pool_create_info(find_queue_families(gpu, surface).graphicsFamily.value());
    VK_CHECK(vkCreateCommandPool(device, &uploadCommandPoolInfo, nullptr, &commandPool));

    // allocate the default command buffer that we will use for the instant commands
    VkCommandBufferAllocateInfo cmdAllocInfo = init::command_buffer_allocate_info(commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &commandBuffer));
}
void utils::UploadContext::cleanup(VkDevice &device)
{
    vkDestroyFence(device, uploadFence, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
}

void utils::UploadContext::immediate_submit(VkDevice &device, VkQueue &gfxQueue,
                                            std::function<void(VkCommandBuffer cmd)> &&function)
{
    VkCommandBuffer cmd = commandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo =
        init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit = init::submit_info(&cmd);

    VK_CHECK(vkQueueSubmit(gfxQueue, 1, &submit, uploadFence));

    vkWaitForFences(device, 1, &uploadFence, true, 9999999999);
    vkResetFences(device, 1, &uploadFence);

    vkResetCommandPool(device, commandPool, 0);
}

} // namespace render

VULKAN_ENGINE_NAMESPACE_END