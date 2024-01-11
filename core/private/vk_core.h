#ifndef VK_CORE
#define VK_CORE

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <shaderc/shaderc.hpp>
#include <vma/vk_mem_alloc.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <map>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>

#define DEBUG_LOG(msg)                 \
	{                                  \
		std::cout << msg << std::endl; \
	}
#define ERR_LOG(msg)                   \
	{                                  \
		std::cerr << msg << std::endl; \
	}
#define VK_CHECK(x)                                                     \
	do                                                                  \
	{                                                                   \
		VkResult err = x;                                               \
		if (err)                                                        \
		{                                                               \
			std::cout << "Detected Vulkan error: " << err << std::endl; \
			abort();                                                    \
		}                                                               \
	} while (0)

// Change name to glm objects
//....
//....
//....
//....
//....
//....
//....
//....


// Shadows
typedef enum ShadowResolutionFlagsBits
{
	VK_LOW_SHADOW_RESOLUTION = 512,
	VK_MEDIUM_SHADOW_RESOLUTION = 1024,
	VK_HIGH_SHADOW_RESOLUTION = 2048,
	VK_ULTRA_SHADOW_RESOLUTION = 4096,
} ShadowResolutionFlagsBits;

// Ahead declaration of some key classes
namespace vke
{
	class Renderer;
}

#endif
