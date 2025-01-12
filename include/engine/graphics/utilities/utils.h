/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <deque>
#include <engine/common.h>
#include <engine/graphics/utilities/initializers.h>
#include <functional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

namespace Utils {

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;
    std::optional<uint32_t> sparseBindingFamily;

    inline bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

QueueFamilyIndices      find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface);


struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)(); // call functors
        }

        deletors.clear();
    }
};
class ManualTimer
{
    std::chrono::high_resolution_clock::time_point t0;
    double                                         timestamp{0.0};

  public:
    void start() {
        t0 = std::chrono::high_resolution_clock::now();
    }
    void stop() {
        timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count() * 1000.0;
    }
    const double& get() {
        return timestamp;
    }
};

struct memory_buffer : public std::streambuf {
    char*  p_start{nullptr};
    char*  p_end{nullptr};
    size_t size;

    memory_buffer(char const* first_elem, size_t size)
        : p_start(const_cast<char*>(first_elem))
        , p_end(p_start + size)
        , size(size) {
        setg(p_start, p_start, p_end);
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
        if (dir == std::ios_base::cur)
            gbump(static_cast<int>(off));
        else
            setg(p_start, (dir == std::ios_base::beg ? p_start : p_end) + off, p_end);
        return gptr() - p_start;
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

struct memory_stream : virtual memory_buffer, public std::istream {
    memory_stream(char const* first_elem, size_t size)
        : memory_buffer(first_elem, size)
        , std::istream(static_cast<std::streambuf*>(this)) {
    }
};

// Function to trim leading and trailing whitespace
std::string trim(const std::string& str);

std::string read_file(const std::string& filePath);

inline std::vector<uint8_t> read_file_binary(const std::string& pathToFile) {
    std::ifstream        file(pathToFile, std::ios::binary);
    std::vector<uint8_t> fileBufferBytes;

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size_t sizeBytes = file.tellg();
        file.seekg(0, std::ios::beg);
        fileBufferBytes.resize(sizeBytes);
        if (file.read((char*)fileBufferBytes.data(), sizeBytes))
            return fileBufferBytes;
    } else
        throw std::runtime_error("could not open binary ifstream to path " + pathToFile);
    return fileBufferBytes;
}

VkPhysicalDeviceProperties get_gpu_properties(VkPhysicalDevice gpu);

VkPhysicalDeviceFeatures get_gpu_features(VkPhysicalDevice gpu);
inline bool              is_instance_extension_supported(const char* extensionName) {
    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for (const auto& extension : extensions)
    {
        if (strcmp(extension.extensionName, extensionName) == 0)
        {
            return true;
        }
    }

    return false;
}

// Check if device extension is supported
inline bool is_device_extension_supported(VkPhysicalDevice physicalDevice, const char* extensionName) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

    for (const auto& extension : extensions)
    {
        if (strcmp(extension.extensionName, extensionName) == 0)
        {
            return true;
        }
    }

    return false;
}

uint32_t find_memory_type(VkPhysicalDevice gpu, uint32_t typeFilter, VkMemoryPropertyFlags properties);

bool check_validation_layer_suport(std::vector<const char*> validationLayers);

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void*                                       pUserData) {

    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult create_debug_utils_messenger_EXT(VkInstance                                instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks*              pAllocator,
                                          VkDebugUtilsMessengerEXT*                 pDebugMessenger);

void destroy_debug_utils_messenger_EXT(VkInstance                   instance,
                                       VkDebugUtilsMessengerEXT     debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);

void log_available_extensions(std::vector<VkExtensionProperties> ext);

void log_available_gpus(std::multimap<int, VkPhysicalDevice> candidates);

Vec3 get_tangent_gram_smidt(Vec3& p1, Vec3& p2, Vec3& p3, glm::vec2& uv1, glm::vec2& uv2, glm::vec2& uv3, Vec3 normal);

}; // namespace Utils
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif