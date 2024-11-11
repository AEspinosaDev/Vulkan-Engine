/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef COMMON_H
#define COMMON_H

#ifdef _WIN32
// Windows-specific includes and definitions
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif __linux__
// Linux-specific includes and definitions
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_XCB_KHR
#include <GLFW/glfw3.h>
#else
#error "Unsupported platform"
#endif
#include <algorithm>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <cstdlib>
#include <fstream>
#include <glm/glm.hpp>
#include <imgui.h>
#include <iostream>
#include <map>
#include <optick.h>
#include <optional>
#include <set>
#include <shaderc/shaderc.hpp>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <vma/vk_mem_alloc.h>

// ENGINE DEFINITIONS

#define ASSERT_PTR(ptr) assert((ptr) && "Pointer is null")

//  #define ENABLE_OPTICK_PROFILING
#ifdef ENABLE_OPTICK_PROFILING
#define PROFILING_EVENT() OPTICK_EVENT()
#define PROFILING_FRAME() OPTICK_FRAME("MainThread");
#else
#define PROFILING_EVENT()
#define PROFILING_FRAME()
#endif

#define _LOG(msg)                                                                                                      \
    { std::cout << "VKEngine log: " << msg << std::endl; }
#define DEBUG_LOG(msg)                                                                                                 \
    { std::cout << "VKEngine debug: " << msg << std::endl; }
#define ERR_LOG(msg)                                                                                                   \
    { std::cerr << "VKEngine error: " << msg << std::endl; }
#define VK_CHECK(x)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        VkResult err = x;                                                                                              \
        if (err)                                                                                                       \
        {                                                                                                              \
            std::cout << "VKEngine detected a Vulkan error: " << err << std::endl;                                     \
            abort();                                                                                                   \
        }                                                                                                              \
    } while (0)

// Namespace define
#define VULKAN_ENGINE_NAMESPACE_BEGIN namespace VKFW {
#define VULKAN_ENGINE_NAMESPACE_END }
#define USING_VULKAN_ENGINE_NAMESPACE using namespace VKFW;

#define VK_MAX_OBJECTS 100
#define VK_MAX_LIGHTS 50

// File terminations
#define PLY "ply"
#define OBJ "obj"
#define FBX "fbx"
#define PNG "png"
#define HDR "hdr"
#define JPG "jpg"
#define HAIR "hair"

#define CUBEMAP_FACES 6

/// Simple exception class, which stores a human-readable error description
class VKException : public std::runtime_error
{
  public:
    template <typename... Args>
    VKException(const char* fmt, const Args&... args)
        : std::runtime_error(fmt) {
    }
};

VULKAN_ENGINE_NAMESPACE_BEGIN

// Mathematics library glm
namespace math = glm;
typedef math::vec4 Vec4;
typedef math::vec3 Vec3;
typedef math::vec2 Vec2;
typedef math::mat4 Mat4;
typedef math::mat3 Mat3;

typedef VkExtent3D Extent3D;
typedef VkExtent2D Extent2D;
typedef VkOffset2D Offset2D;

typedef enum ObjectType
{
    MESH   = 0,
    LIGHT  = 1,
    CAMERA = 2,
    VOLUME = 3,
    SKYBOX = 4,
    OTHER

} ObjectType;

/**
Support for some presets of commercial game engines
*/
typedef enum MaskType
{
    NO_MASK       = -1,
    UNITY_HDRP    = 0,
    UNREAL_ENGINE = 1,
    UNITY_URP     = 2
} MaskType;

typedef enum CullingMode
{
    _FRONT = VK_CULL_MODE_FRONT_BIT,
    _BACK  = VK_CULL_MODE_BACK_BIT,
    _NO_CULLING = VK_CULL_MODE_NONE,
} CullingMode;

typedef enum BufferingType
{
    _UNIQUE    = 1,
    _DOUBLE    = 2,
    _TRIPLE    = 3,
    _QUADRUPLE = 4
} BufferingType;

typedef enum MSAASamples
{
    _NONE    = VK_SAMPLE_COUNT_1_BIT,
    MSAA_x4  = VK_SAMPLE_COUNT_4_BIT,
    MSAA_x8  = VK_SAMPLE_COUNT_8_BIT,
    MSAA_x16 = VK_SAMPLE_COUNT_16_BIT,
    MSAA_x32 = VK_SAMPLE_COUNT_32_BIT,
} MSAASamples;

typedef enum ShadowResolution
{
    VERY_LOW = 256,
    LOW      = 512,
    MEDIUM   = 1024,
    HIGH     = 2048,
    ULTRA    = 4096
} ShadowResolution;

typedef enum ControllerMovementType
{
    ORBITAL,
    WASD,
} ControllerMovementType;

typedef enum VertexAttributeType
{
    POSITION = 0,
    NORMAL   = 1,
    TANGENT  = 2,
    UV       = 3,
    COLOR    = 4
} VertexAttributeType0;

typedef enum DescriptorLayoutType
{
    GLOBAL_LAYOUT         = 0,
    OBJECT_LAYOUT         = 1,
    OBJECT_TEXTURE_LAYOUT = 2,
    G_BUFFER_LAYOUT       = 3
} DescriptorLayoutType;

typedef enum TextureFilterType
{
    NEAREST = VK_FILTER_NEAREST,
    LINEAR  = VK_FILTER_LINEAR,
    CUBIC   = VK_FILTER_CUBIC_EXT,
    MAX     = VK_FILTER_MAX_ENUM
} TextureFilterType;
typedef enum ColorFormatType
{
    SR_8      = VK_FORMAT_R8_SRGB,       // Red
    SRG_8     = VK_FORMAT_R8G8_SRGB,     // Red Green
    SRGB_8    = VK_FORMAT_R8G8B8_SRGB,   // RGB
    SRGBA_8   = VK_FORMAT_R8G8B8A8_SRGB, // RGB with Alpha
    SBGRA_8   = VK_FORMAT_B8G8R8A8_SRGB, // Other order
    SRG_16F   = VK_FORMAT_R16G16_SFLOAT,
    SRG_32F   = VK_FORMAT_R32G32_SFLOAT,
    SRGB_16F  = VK_FORMAT_R16G16B16_SFLOAT,
    SRGB_32F  = VK_FORMAT_R32G32B32_SFLOAT,
    SRGBA_16F = VK_FORMAT_R16G16B16A16_SFLOAT, // HDR precission 16
    SRGBA_32F = VK_FORMAT_R32G32B32A32_SFLOAT, // HDR precission 32
    RGBA_8U   = VK_FORMAT_R8G8B8A8_UNORM,
} ColorFormatType;
typedef enum DepthFormatType
{
    D16F = VK_FORMAT_D16_UNORM,
    D32F = VK_FORMAT_D32_SFLOAT
} DepthFormatType;
typedef enum TextureAdressModeType
{
    REPEAT        = VK_SAMPLER_ADDRESS_MODE_REPEAT, // Repeat the texture when going beyond the image dimensions.
    MIRROR_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, // Like repeat, but inverts the coordinates to mirror the
                                                             // image when going beyond the dimensions.
    EDGE_CLAMP = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, // Take the color of the edge closest to the coordinate beyond
                                                        // the image dimensions.
    MIRROR_EDGE_CLAMP = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, // Like clamp to edge, but instead uses the edge
                                                                      // opposite to the closest edge.
    BORDER_CLAMP = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER // Return a solid color when sampling beyond the dimensions
                                                           // of the image.
} TextureAdressModeType;

typedef enum PanelWidgetFlags
{

    None                      = ImGuiWindowFlags_None,
    NoTitleBar                = ImGuiWindowFlags_NoTitleBar,
    NoResize                  = ImGuiWindowFlags_NoResize,
    NoMove                    = ImGuiWindowFlags_NoMove,
    NoScrollbar               = ImGuiWindowFlags_NoScrollbar,
    NoScrollWithMouse         = ImGuiWindowFlags_NoScrollWithMouse,
    NoCollapse                = ImGuiWindowFlags_NoCollapse,
    AlwaysAutoResize          = ImGuiWindowFlags_AlwaysAutoResize,
    NoBackground              = ImGuiWindowFlags_NoBackground,
    NoSavedSettings           = ImGuiWindowFlags_NoSavedSettings,
    NoMouseInputs             = ImGuiWindowFlags_NoMouseInputs,
    MenuBar                   = ImGuiWindowFlags_MenuBar,
    HorizontalScrollbar       = ImGuiWindowFlags_HorizontalScrollbar,
    NoFocusOnAppearing        = ImGuiWindowFlags_NoFocusOnAppearing,
    NoBringToFrontOnFocus     = ImGuiWindowFlags_NoBringToFrontOnFocus,
    AlwaysVerticalScrollbar   = ImGuiWindowFlags_AlwaysVerticalScrollbar,
    AlwaysHorizontalScrollbar = ImGuiWindowFlags_AlwaysHorizontalScrollbar,
    NoNavInputs               = ImGuiWindowFlags_NoNavInputs,
    NoNavFocus                = ImGuiWindowFlags_NoNavFocus,
    UnsavedDocument           = ImGuiWindowFlags_UnsavedDocument,
    NoNav                     = ImGuiWindowFlags_NoNav,
    NoDecoration              = ImGuiWindowFlags_NoDecoration,
    NoInputs                  = ImGuiWindowFlags_NoInputs,

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
    POINT       = 0,
    DIRECTIONAL = 1,
    SPOT        = 2,
    AREA        = 3
} LightType;

typedef enum ShadowType
{
    BASIC     = 0, // Classic shadow mapping
    VSM       = 1, // Variance shadow mapping
    RAYTRACED = 2, // Raytraced shadow
} ShadowType;

typedef enum GuiColorProfileType
{
    DARK    = 0,
    BRIGHT  = 1,
    CLASSIC = 2,
    CUSTOM  = 3
} GuiColorProfileType;

typedef enum VolumeType
{
    SPHERE = 0,
    AABB   = 1,
    OBB    = 2,
} VolumeType;

typedef enum SyncType
{
    NONE_SYNC      = VK_PRESENT_MODE_IMMEDIATE_KHR, // No framerate cap (POTENTIAL TEARING)
    MAILBOX_SYNC   = VK_PRESENT_MODE_MAILBOX_KHR,   // Triple buffering (Better V-Sync)
    V_SYNC         = VK_PRESENT_MODE_FIFO_KHR,      // Classic V-Sync
    RELAXED_V_SYNC = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
    // V-Sync with a wait time. If wait time is not enough potential tearing
} SyncType;

typedef enum RendererType
{
    FORWARD_RENDERER  = 0,
    DEFERRED_RENDERER = 1,
} RendererType;

typedef enum AmbientOcclusionType
{
    SSAO  = 0,
    USSAO = 1,
    DSSAO = 2
} AmbientOcclusionType;

typedef enum WindowingSystem
{
    GLFW = 0,
    SDL  = 1
} WindowingSystem;
enum QueueType
{
    GRAPHIC = 0,
    PRESENT = 1,
    COMPUTE = 2,
    RT      = 3
};
enum AttachmentType
{
    COLOR_ATTACHMENT   = 0,
    DEPTH_ATTACHMENT   = 1,
    RESOLVE_ATTACHMENT = 2,
};

enum ShaderStageType
{
    NONE_STAGE      = -1,
    VERTEX          = 0,
    FRAGMENT        = 1,
    GEOMETRY        = 2,
    TESS_CONTROL    = 3,
    TESS_EVALUATION = 4
};

enum UniformDataType
{
    UNIFORM_BUFFER         = 0,
    DYNAMIC_UNIFORM_BUFFER = 1,
    COMBINED_IMAGE_SAMPLER = 2
};

// Sample count enum: to represent sample counts in a clearer way
enum class SampleCount
{
    SAMPLE_COUNT_1 = 0,
    SAMPLE_COUNT_2,
    SAMPLE_COUNT_4,
    SAMPLE_COUNT_8,
    SAMPLE_COUNT_16,
    SAMPLE_COUNT_32,
    SAMPLE_COUNT_64,
    MAX_SAMPLE_COUNT
};

// ImageUsageFlags enum: to represent image usage flags in a more readable way
enum class ImageUsageFlags
{
    COLOR_ATTACHMENT = 0,
    DEPTH_STENCIL_ATTACHMENT,
    SAMPLED,
    STORAGE,
    TRANSFER_SRC,
    TRANSFER_DST,
    SHADER_READ_ONLY,
    TRANSFER,
    MAX_IMAGE_USAGE
};

// ImageLayout enum: representing possible image layouts in Vulkan
enum class ImageLayoutType
{
    UNDEFINED = 0,
    COLOR_ATTACHMENT_OPTIMAL,
    DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    SHADER_READ_ONLY_OPTIMAL,
    TRANSFER_SRC_OPTIMAL,
    TRANSFER_DST_OPTIMAL,
    GENERAL,
    MAX_IMAGE_LAYOUT
};

// ImageViewType enum: representing image view types
enum class ImageViewType
{
    TYPE_1D = 0,
    TYPE_2D,
    TYPE_3D,
    TYPE_CUBE,
    TYPE_1D_ARRAY,
    TYPE_2D_ARRAY,
    TYPE_CUBE_ARRAY,
    MAX_IMAGE_VIEW_TYPE
};

// FilterType enum: for Vulkan texture filters
enum class FilterType
{
    NEAREST = 0,
    LINEAR,
    CUBIC,
    MAX_FILTER_TYPE
};

// AddressMode enum: for sampler address modes in Vulkan
enum class AddressMode
{
    REPEAT = 0,
    MIRROR_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    MIRROR_CLAMP_TO_EDGE,
    MAX_ADDRESS_MODE
};

// ClearValue enum: to set clear values for attachments
enum class ClearValueType
{
    COLOR = 0,
    DEPTH_STENCIL,
    MAX_CLEAR_VALUE
};

// SubPassDependencyFlags enum: for Vulkan subpass dependency flags
enum class SubPassDependencyFlags
{
    BY_REGION = 0,
    MAX_SUBPASS_DEPENDENCY_FLAGS
};

// PipelineStageFlags enum: for pipeline stages in Vulkan
enum class PipelineStageFlags
{
    TOP_OF_PIPE = 0,
    BOTTOM_OF_PIPE,
    COLOR_ATTACHMENT_OUTPUT,
    EARLY_FRAGMENT_TESTS,
    LATE_FRAGMENT_TESTS,
    ALL_GRAPHICS,
    TRANSFER,
    COMPUTE_SHADER,
    ALL_COMMANDS,
    MAX_PIPELINE_STAGE_FLAGS
};

// AccessFlags enum: to represent access types for Vulkan resources
enum class AccessFlags
{
    ACCESS_NONE = 0,
    COLOR_ATTACHMENT_READ,
    COLOR_ATTACHMENT_WRITE,
    DEPTH_STENCIL_ATTACHMENT_READ,
    DEPTH_STENCIL_ATTACHMENT_WRITE,
    TRANSFER_READ,
    TRANSFER_WRITE,
    SHADER_READ,
    SHADER_WRITE,
    MAX_ACCESS_FLAGS
};

enum class TextureFormatType
{
    COLOR_FORMAT  = 0,
    NORMAL_FORMAT = 1,
    DEPTH_FORMAT  = 2,
    HDR_FORMAT    = 3,
};

enum class BindingType
{
    GRAPHIC_BINDING    = VK_PIPELINE_BIND_POINT_GRAPHICS,
    COMPUTE_BINDING    = VK_PIPELINE_BIND_POINT_COMPUTE,
    RAYTRACING_BINDING = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
};

VULKAN_ENGINE_NAMESPACE_END

#endif
