/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <engine/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

namespace Booter {
#pragma region Instance
// Instance
VkInstance create_instance(const char* appName, const char* engineName, bool validation, std::vector<const char*> validationLayers);

#pragma region Validation
// Validation Logger
VkDebugUtilsMessengerEXT                     create_debug_messenger(VkInstance instance);
void                                         populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void*                                       pUserData) {

    DEBUG_LOG("(Validation Layer) " << pCallbackData->pMessage);

    return VK_FALSE;
}
VkResult create_debug_utils_messenger_EXT(VkInstance                                instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks*              pAllocator,
                                          VkDebugUtilsMessengerEXT*                 pDebugMessenger);
void     destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
#pragma region GPU
// GPU
VkPhysicalDevice pick_graphics_card_device(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*> extensions);
int              rate_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> extensions);
bool             check_device_extension_support(VkPhysicalDevice device, std::vector<const char*> extensions);
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};
SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> transferFamily;
    std::optional<uint32_t> sparseBindingFamily;

    inline bool isComplete(bool headless = false) const {

        return !headless ? graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value()
                         : graphicsFamily.has_value() && computeFamily.has_value();
    }
};
QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

#pragma region Device
// Logical Device
VkDevice create_logical_device(std::unordered_map<QueueType, VkQueue>& queues,
                               VkPhysicalDevice                        gpu,
                               VkPhysicalDeviceFeatures                features,
                               VkSurfaceKHR                            surface,
                               bool                                    validation,
                               std::vector<const char*>                validationLayers);
#pragma region VMA
// VMA
VmaAllocator setup_memory(VkInstance instance, VkDevice device, VkPhysicalDevice gpu);

#pragma region Utils
// Utilities
std::vector<const char*> get_required_extensions(bool validation);
bool                     check_validation_layer_suport(std::vector<const char*> validationLayers);
bool                     is_instance_extension_supported(const char* extensionName);
bool                     is_device_extension_supported(VkPhysicalDevice physicalDevice, const char* extensionName);
void                     log_available_extensions(std::vector<VkExtensionProperties> ext);
void                     log_available_gpus(std::multimap<int, VkPhysicalDevice> candidates);

}; // namespace Booter

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif