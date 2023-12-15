#ifndef VK_UTILS_H
#define VK_UTILS_H

#include "vk_core.h"
#include <deque>
#include <functional>

namespace vke
{

	namespace vkutils
	{
		struct DeletionQueue
		{
			std::deque<std::function<void()>> deletors;

			void push_function(std::function<void()> &&function)
			{
				deletors.push_back(function);
			}

			void flush()
			{
				// reverse iterate the deletion queue to execute all the functions
				for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
				{
					(*it)(); // call functors
				}

				deletors.clear();
			}
		};

		struct EventDispatcher
		{

			std::vector<std::function<void()>> functions;

			void push_function(std::function<void()> &&function)
			{
				functions.push_back(function);
			}

			void dispatch()
			{
				// reverse iterate the deletion queue to execute all the functions
				for (auto it = functions.rbegin(); it != functions.rend(); it++)
				{
					(*it)(); // call functors
				}

				functions.clear();
			}
		};

		size_t pad_uniform_buffer_size(size_t originalSize, VkPhysicalDevice gpu);

		uint32_t find_memory_type(VkPhysicalDevice gpu, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		bool check_validation_layer_suport(std::vector<const char *> validationLayers);

		void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

		inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *pUserData)
		{

			std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}

		VkResult create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

		void destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

		void log_available_extensions(std::vector<VkExtensionProperties> ext);

		void log_available_gpus(std::multimap<int, VkPhysicalDevice> candidates);

		template <typename T, typename... Rest>
		void hash_combine(std::size_t &seed, const T &v, const Rest &...rest)
		{
			seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			(hash_combine(seed, rest), ...);
		}

	};

}
#endif