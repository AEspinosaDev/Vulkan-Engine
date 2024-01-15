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

namespace vke
{
	// Change name to glm objects
	//....
	//....

	typedef enum BufferingType
	{
		_UNIQUE = 1,
		_DOUBLE = 2,
		_TRIPLE = 3,
		_QUADRUPLE = 4
	} BufferingType;

	typedef enum AntialiasingType
	{
		_NONE = VK_SAMPLE_COUNT_1_BIT,
		MSAA_x4 = VK_SAMPLE_COUNT_4_BIT,
		MSAA_x8 = VK_SAMPLE_COUNT_8_BIT,
		MSAA_x16 = VK_SAMPLE_COUNT_16_BIT,
		MSAA_x32 = VK_SAMPLE_COUNT_32_BIT
	} AntialiasingType;

	typedef enum ShadowResolution
	{
		LOW = 512,
		MEDIUM = 1024,
		HIGH = 2048,
		ULTRA = 4096
	} ShadowResolution;

	typedef enum ControllerMovementType
	{
		ORBITAL,
		WASD,
	} ControllerMovementType;

	typedef enum VertexAttributeType
	{
		POSITION = 0,
		NORMAL = 1,
		TANGENT = 2,
		UV = 3,
		COLOR = 4
	} VertexAttributeType0;

	typedef enum DescriptorLayoutType
	{
		GLOBAL_LAYOUT = 0,
		OBJECT_LAYOUT = 1,
		TEXTURE_LAYOUT = 2
	} DescriptorLayoutType;

	typedef enum TextureFilterType
	{
		NEAREST = VK_FILTER_NEAREST,
		LINEAR = VK_FILTER_LINEAR,
		CUBIC = VK_FILTER_CUBIC_EXT,
		MAX = VK_FILTER_MAX_ENUM
	} TextureFilterType;
	typedef enum TextureFormatType
	{
		SRGBA_8 = VK_FORMAT_R8G8B8A8_SRGB,
		SRGB_8 = VK_FORMAT_R8G8B8_SRGB,
		RGBA_8,
		RGB_8,
		SRGBA_16,
		SRGB_16,
		RGBA_16,
		RGB_16,
	} TextureFormatType;
	typedef enum TextureAdressModeType
	{
		REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,						  // Repeat the texture when going beyond the image dimensions.
		MIRROR_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,		  // Like repeat, but inverts the coordinates to mirror the image when going beyond the dimensions.
		EDGE_CLAMP = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,				  // Take the color of the edge closest to the coordinate beyond the image dimensions.
		MIRROR_EDGE_CLAMP = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, // Like clamp to edge, but instead uses the edge opposite to the closest edge.
		BORDER_CLAMP = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER			  // Return a solid color when sampling beyond the dimensions of the image.
	} TextureAdressModeType;

	// Ahead declaration of some key classes
	class Renderer;
}

#endif
