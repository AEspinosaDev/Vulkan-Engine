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
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

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

	typedef enum PanelWidgetFlags
	{

		None = ImGuiWindowFlags_None,
		NoTitleBar = ImGuiWindowFlags_NoTitleBar,								// Disable title-bar
		NoResize = ImGuiWindowFlags_NoResize,									// Disable user resizing with the lower-right grip
		NoMove = ImGuiWindowFlags_NoMove,										// Disable user moving the window
		NoScrollbar = ImGuiWindowFlags_NoScrollbar,								// Disable scrollbars (window can still scroll with mouse or programmatically)
		NoScrollWithMouse = ImGuiWindowFlags_NoScrollWithMouse,					// Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
		NoCollapse = ImGuiWindowFlags_NoCollapse,								// Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
		AlwaysAutoResize = ImGuiWindowFlags_AlwaysAutoResize,					// Resize every window to its content every frame
		NoBackground = ImGuiWindowFlags_NoBackground,							// Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
		NoSavedSettings = ImGuiWindowFlags_NoSavedSettings,						// Never load/save settings in .ini file
		NoMouseInputs = ImGuiWindowFlags_NoMouseInputs,							// Disable catching mouse, hovering test with pass through.
		MenuBar = ImGuiWindowFlags_MenuBar,										// Has a menu-bar
		HorizontalScrollbar = ImGuiWindowFlags_HorizontalScrollbar,				// Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
		NoFocusOnAppearing = ImGuiWindowFlags_NoFocusOnAppearing,				// Disable taking focus when transitioning from hidden to visible state
		NoBringToFrontOnFocus = ImGuiWindowFlags_NoBringToFrontOnFocus,			// Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
		AlwaysVerticalScrollbar = ImGuiWindowFlags_AlwaysVerticalScrollbar,		// Always show vertical scrollbar (even if ContentSize.y < Size.y)
		AlwaysHorizontalScrollbar = ImGuiWindowFlags_AlwaysHorizontalScrollbar, // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
		NoNavInputs = ImGuiWindowFlags_NoNavInputs,								// No gamepad/keyboard navigation within the window
		NoNavFocus = ImGuiWindowFlags_NoNavFocus,								// No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
		UnsavedDocument = ImGuiWindowFlags_UnsavedDocument,						// Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
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

	// Ahead declaration of some key classes
	class Renderer;
}

#endif
