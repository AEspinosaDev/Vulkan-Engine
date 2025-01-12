/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <engine/graphics/utilities/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

namespace Booter
{
// Instance
VkInstance create_instance(const char *appName, const char *engineName, bool validation,
                           std::vector<const char *> validationLayers);
std::vector<const char *> get_required_extensions(bool validation);
// Logger
VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance instance);
// GPU
VkPhysicalDevice pick_graphics_card_device(VkInstance instance, VkSurfaceKHR surface,
                                           std::vector<const char *> extensions);

int rate_device_suitability(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char *> extensions);
bool check_device_extension_support(VkPhysicalDevice device, std::vector<const char *> extensions);

// Logical Device
VkDevice create_logical_device(std::unordered_map<QueueType, VkQueue> &queues, VkPhysicalDevice gpu,
                               VkPhysicalDeviceFeatures features, VkSurfaceKHR surface, bool validation,
                               std::vector<const char *> validationLayers);
// VMA
VmaAllocator setup_memory(VkInstance instance, VkDevice device, VkPhysicalDevice gpu);


}; // namespace Booter

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif