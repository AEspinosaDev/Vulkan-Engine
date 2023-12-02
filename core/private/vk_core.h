#ifndef VK_CORE_h
#define VK_CORE_h


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
#include <fstream>
#include <sstream>
#include <algorithm> 

//Utils
#define DEBUG_LOG(msg) { std::cout << msg << std::endl;}
#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)


//Ahead declaration of some key classes	
namespace vke{
	class Renderer;
}

#endif



