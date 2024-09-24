/*
	This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

	MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMMON_H
#define COMMON_H

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
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

// Convenience functions definitions
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

// Namespace define
#define VULKAN_ENGINE_NAMESPACE_BEGIN \
	namespace vke                     \
	{
#define VULKAN_ENGINE_NAMESPACE_END }
#define USING_VULKAN_ENGINE_NAMESPACE using namespace vke;

#define VK_MAX_OBJECTS 100
#define VK_MAX_LIGHTS 50

// File terminations
#define PLY "ply"
#define OBJ "obj"
#define FBX "fbx"
#define PNG "png"
#define JPG "jpg"

/// Simple exception class, which stores a human-readable error description
class VKException : public std::runtime_error
{
public:
	template <typename... Args>
	VKException(const char *fmt, const Args &...args)
		: std::runtime_error(fmt) {}
};

VULKAN_ENGINE_NAMESPACE_BEGIN

// Mathematics library glm
namespace math = glm;
typedef math::vec4 Vec4;
typedef math::vec3 Vec3;
typedef math::vec2 Vec2;
typedef math::mat4 Mat4;
typedef math::mat3 Mat3;

typedef enum ObjectType
{
	MESH = 0,
	LIGHT = 1,
	CAMERA = 2,
	OTHER

} ObjectType;

/**
Support for some presets of commercial game engines
*/
typedef enum MaskType
{
	NO_MASK = -1,
	UNITY_HDRP = 0,
	UNREAL_ENGINE = 1,
	UNITY_URP = 2
} MaskType;

typedef enum CullingMode
{
	_FRONT = VK_CULL_MODE_FRONT_BIT,
	_BACK = VK_CULL_MODE_BACK_BIT
} CullingMode;

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
	MSAA_x32 = VK_SAMPLE_COUNT_32_BIT,
	FXAA = 0
} AntialiasingType;

typedef enum ShadowResolution
{
	VERY_LOW = 256,
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
	OBJECT_TEXTURE_LAYOUT = 2,
	G_BUFFER_LAYOUT = 3
} DescriptorLayoutType;

typedef enum TextureFilterType
{
	NEAREST = VK_FILTER_NEAREST,
	LINEAR = VK_FILTER_LINEAR,
	CUBIC = VK_FILTER_CUBIC_EXT,
	MAX = VK_FILTER_MAX_ENUM
} TextureFilterType;
typedef enum ColorFormatType
{
	SRGBA_8 = VK_FORMAT_R8G8B8A8_SRGB,
	SRGB_8 = VK_FORMAT_R8G8B8_SRGB,
	SBGRA_8 = VK_FORMAT_B8G8R8A8_SRGB,
	RGBA_8,
	RGB_8,
	SRGBA_16,
	SRGB_16,
	RGBA_16,
	RGB_16,
} ColorFormatType;
typedef enum DepthFormatType
{
	D16F = VK_FORMAT_D16_UNORM,
	D32F = VK_FORMAT_D32_SFLOAT
} DepthFormatType;
typedef enum TextureAdressModeType
{
	REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,						  // Repeat the texture when going beyond the image dimensions.
	MIRROR_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,		  // Like repeat, but inverts the coordinates to mirror the image when going beyond the dimensions.
	EDGE_CLAMP = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,				  // Take the color of the edge closest to the coordinate beyond the image dimensions.
	MIRROR_EDGE_CLAMP = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, // Like clamp to edge, but instead uses the edge opposite to the closest edge.
	BORDER_CLAMP = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER			  // Return a solid color when sampling beyond the dimensions of the image.
} TextureAdressModeType;

typedef enum PanelWidgetFlags
{

	None = ImGuiWindowFlags_None,
	NoTitleBar = ImGuiWindowFlags_NoTitleBar,
	NoResize = ImGuiWindowFlags_NoResize,
	NoMove = ImGuiWindowFlags_NoMove,
	NoScrollbar = ImGuiWindowFlags_NoScrollbar,
	NoScrollWithMouse = ImGuiWindowFlags_NoScrollWithMouse,
	NoCollapse = ImGuiWindowFlags_NoCollapse,
	AlwaysAutoResize = ImGuiWindowFlags_AlwaysAutoResize,
	NoBackground = ImGuiWindowFlags_NoBackground,
	NoSavedSettings = ImGuiWindowFlags_NoSavedSettings,
	NoMouseInputs = ImGuiWindowFlags_NoMouseInputs,
	MenuBar = ImGuiWindowFlags_MenuBar,
	HorizontalScrollbar = ImGuiWindowFlags_HorizontalScrollbar,
	NoFocusOnAppearing = ImGuiWindowFlags_NoFocusOnAppearing,
	NoBringToFrontOnFocus = ImGuiWindowFlags_NoBringToFrontOnFocus,
	AlwaysVerticalScrollbar = ImGuiWindowFlags_AlwaysVerticalScrollbar,
	AlwaysHorizontalScrollbar = ImGuiWindowFlags_AlwaysHorizontalScrollbar,
	NoNavInputs = ImGuiWindowFlags_NoNavInputs,
	NoNavFocus = ImGuiWindowFlags_NoNavFocus,
	UnsavedDocument = ImGuiWindowFlags_UnsavedDocument,
	NoNav = ImGuiWindowFlags_NoNav,
	NoDecoration = ImGuiWindowFlags_NoDecoration,
	NoInputs = ImGuiWindowFlags_NoInputs,

} PanelWidgetFlags;

typedef enum TextWidgetType
{
	SIMPLE,
	COLORIZED,
	WARPED,
	BULLET,
} TextWidgetType;

typedef enum LightType
{
	POINT = 0,
	DIRECTIONAL = 1,
	SPOT = 2,
	AREA = 3
} LightType;

typedef enum GuiColorProfileType
{
	DARK = 0,
	BRIGHT = 1,
	CLASSIC = 2,
	CUSTOM = 3
} GuiColorProfileType;

typedef enum VolumeType
{
	SPHERE = 0,
	AABB = 1,
	OBB = 2,
} VolumeType;

typedef enum SyncType
{
	NONE = VK_PRESENT_MODE_IMMEDIATE_KHR,			   // No framerate cap (POTENTIAL TEARING)
	MAILBOX_SYNC = VK_PRESENT_MODE_MAILBOX_KHR,		   // Triple buffering (Better V-Sync)
	V_SYNC = VK_PRESENT_MODE_FIFO_KHR,				   // Classic V-Sync
	RELAXED_V_SYNC = VK_PRESENT_MODE_FIFO_RELAXED_KHR, // V-Sync with a wait time. If wait time is not enough potential tearing
} SyncType;

typedef enum RendererType
{
	TFORWARD = 0,
	TDEFERRED = 1,
} RendererType;

typedef enum AmbientOcclusionType
{
	SSAO = 0,
	USSAO = 1,
	DSSAO = 2
} AmbientOcclusionType;

/*
Vulkan API extension data
*/
struct Extension
{
	const char *name{nullptr}; //Necesary
	bool optional{false};
	uint32_t version{0};

	Extension(const char *name, bool isOptional = false, void *pointerFeatureStruct = nullptr, uint32_t checkVersion = 0)
		: name(name), optional(isOptional), version(checkVersion)
	{
	}
};

// Ahead declaration of some key classes
class Renderer;

VULKAN_ENGINE_NAMESPACE_END

#endif
