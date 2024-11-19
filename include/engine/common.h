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
// #include <Volk/volk.h>
// #define VK_NO_PROTOTYPES
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
    {                                                                                                                  \
        std::cout << "VKEngine log: " << msg << std::endl;                                                             \
    }
#define DEBUG_LOG(msg)                                                                                                 \
    {                                                                                                                  \
        std::cout << "VKEngine debug: " << msg << std::endl;                                                           \
    }
#define ERR_LOG(msg)                                                                                                   \
    {                                                                                                                  \
        std::cerr << "VKEngine error: " << msg << std::endl;                                                           \
    }
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
class VKFW_Exception : public std::runtime_error
{
  public:
    template <typename... Args>
    VKFW_Exception(const char* fmt, const Args&... args)
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

typedef VkExtent3D   Extent3D;
typedef VkExtent2D   Extent2D;
typedef VkOffset2D   Offset2D;
typedef VkClearValue ClearValue;

enum class ObjectType
{
    MESH   = 0,
    LIGHT  = 1,
    CAMERA = 2,
    VOLUME = 3,
    SKYBOX = 4,
    OTHER

};
/**
Support for some presets of commercial game engines
*/
enum class MaskType
{
    NO_MASK       = -1,
    UNITY_HDRP    = 0,
    UNREAL_ENGINE = 1,
    UNITY_URP     = 2
};

typedef enum CullingMode
{
    FRONT_CULLING = VK_CULL_MODE_FRONT_BIT,
    BACK_CULLING  = VK_CULL_MODE_BACK_BIT,
    NO_CULLING    = VK_CULL_MODE_NONE,
} CullingMode;
enum class BufferingType
{
    UNIQUE = 1,
    DOUBLE = 2,
    TRIPLE = 3,
    QUAD   = 4
};
enum class MSAASamples
{
    x1  = 1,
    x4  = 4,
    x8  = 8,
    x16 = 16,
    x32 = 32,
};
enum class ShadowResolution
{
    VERY_LOW = 256,
    LOW      = 512,
    MEDIUM   = 1024,
    HIGH     = 2048,
    ULTRA    = 4096
};

enum class SyncType
{
    NONE             = 0, // No framerate cap (POTENTIAL TEARING)
    MAILBOX          = 1, // Triple buffering (Better V-Sync)
    VERTICAL         = 2, // Classic V-Sync
    RELAXED_VERTICAL = 3,
    // V-Sync with a wait time. If wait time is not enough potential tearing
};

enum class ControllerMovementType
{
    ORBITAL,
    WASD,
};
enum class LightType
{
    POINT       = 0,
    DIRECTIONAL = 1,
    SPOT        = 2,
    AREA        = 3
};

typedef enum VertexAttributeType
{
    POSITION_ATTRIBUTE = 0,
    NORMAL_ATTRIBUTE   = 1,
    TANGENT_ATTRIBUTE  = 2,
    UV_ATTRIBUTE       = 3,
    COLOR_ATTRIBUTE    = 4
} VertexAttributeType;
typedef enum ShadowType
{
    BASIC_SHADOW     = 0, // Classic shadow mapping
    VSM_SHADOW       = 1, // Variance shadow mapping
    RAYTRACED_SHADOW = 2, // Raytraced shadow
} ShadowType;
typedef enum VolumeType
{
    SPHERE_VOLUME = 0,
    AABB_VOLUME   = 1,
    OBB_VOLUME    = 2,
} VolumeType;

typedef enum RendererType
{
    FORWARD_RENDERER  = 0,
    DEFERRED_RENDERER = 1,
} RendererType;

typedef enum WindowingSystem
{
    GLFW = 0,
    SDL  = 1
} WindowingSystem;
enum QueueType
{
    GRAPHIC_QUEUE = 0,
    PRESENT_QUEUE = 1,
    COMPUTE_QUEUE = 2,
    RT_QUEUE      = 3
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
    TESS_EVALUATION = 4,
    ALL_STAGES      = 5
};
enum UniformDataType
{
    UNIFORM_BUFFER         = 0,
    DYNAMIC_UNIFORM_BUFFER = 1,
    COMBINED_IMAGE_SAMPLER = 2,
    ACCELERATION_STRUCTURE = 3,
};
enum BorderColor
{
    FLOAT_TRANSPARENT_BLACK = 0,
    INT_TRANSPARENT_BLACK,
    FLOAT_OPAQUE_BLACK,
    INT_OPAQUE_BLACK,
    FLOAT_OPAQUE_WHITE,
    INT_OPAQUE_WHITE
};
typedef enum DescriptorLayoutType
{
    GLOBAL_LAYOUT         = 0,
    OBJECT_LAYOUT         = 1,
    OBJECT_TEXTURE_LAYOUT = 2,
    G_BUFFER_LAYOUT       = 3
} DescriptorLayout;
typedef enum ColorFormatTypeFlagBits
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
    DEPTH_16F = VK_FORMAT_D16_UNORM,
    DEPTH_32F = VK_FORMAT_D32_SFLOAT
} ColorFormatType;
typedef enum MipmapModeFlagsBits
{
    MIPMAP_NEAREST = 0x00000001,
    MIPMAP_LINEAR  = 0x00000002
} MipmapMode;
typedef enum ImageLayoutFlagBits
{
    LAYOUT_UNDEFINED                        = 0x00000000,
    LAYOUT_COLOR_ATTACHMENT_OPTIMAL         = 0x00000001,
    LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 0x00000002,
    LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL  = 0x00000003,
    LAYOUT_SHADER_READ_ONLY_OPTIMAL         = 0x00000004,
    LAYOUT_TRANSFER_SRC_OPTIMAL             = 0x00000005,
    LAYOUT_TRANSFER_DST_OPTIMAL             = 0x00000006,
    LAYOUT_PRESENT                          = 0x00000007,
    LAYOUT_GENERAL                          = 0x00000008,
} ImageLayout;
typedef enum ImageAspectFlagBits
{
    ASPECT_COLOR    = 0x00000001,
    ASPECT_DEPTH    = 0x00000002,
    ASPECT_STENCIL  = 0x00000003,
    ASPECT_METADATA = 0x00000004,
} ImageAspect;
// Texture type, used for image views also
typedef enum TextureTypeFlagBits
{
    TEXTURE_1D         = 0x00000000,
    TEXTURE_1D_ARRAY   = 0x00000001,
    TEXTURE_2D         = 0x00000002,
    TEXTURE_2D_ARRAY   = 0x00000003,
    TEXTURE_3D         = 0x00000004,
    TEXTURE_CUBE       = 0x00000005,
    TEXTURE_CUBE_ARRAY = 0x00000006,
} TextureType;
// FilterType enum: for Vulkan texture filters
typedef enum FilterTypeFlagBits
{
    FILTER_NEAREST = 0x00000001,
    FILTER_LINEAR  = 0x00000002,
    FILTER_CUBIC   = 0x00000003,
} FilterType;
typedef enum AddressModeFlagBits
{
    ADDRESS_MODE_REPEAT               = 0x00000001,
    ADDRESS_MODE_MIRROR_REPEAT        = 0x00000002,
    ADDRESS_MODE_CLAMP_TO_EDGE        = 0x00000003,
    ADDRESS_MODE_CLAMP_TO_BORDER      = 0x00000004,
    ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 0x00000005,
} AddressMode;
typedef enum class ClearValueTypeFlagBits
{
    CLEAR_COLOR         = 0x00000001,
    CLEAR_DEPTH_STENCIL = 0x00000002,
} ClearValueType;
typedef enum SubPassDependencyTypeFlagsBits
{
    SUBPASS_DEPENDENCY_NONE      = 0x00000000,
    SUBPASS_DEPENDENCY_BY_REGION = 0x00000001
} SubPassDependencyType;
typedef enum PipelineStageFlagsBits
{
    TOP_OF_PIPE_STAGE             = 0x00000001,
    BOTTOM_OF_PIPE_STAGE          = 0x00000002,
    COLOR_ATTACHMENT_OUTPUT_STAGE = 0x00000003,
    EARLY_FRAGMENT_TESTS_STAGE    = 0x00000004,
    LATE_FRAGMENT_TESTS_STAGE     = 0x00000005,
    ALL_GRAPHICS_STAGE            = 0x00000006,
    TRANSFER_STAGE                = 0x00000007,
    COMPUTE_SHADER_STAGE          = 0x00000008,
    FRAGMENT_SHADER_STAGE         = 0x00000009,
    ALL_COMMANDS_STAGE            = 0x00000010,
} PipelineStage;
typedef enum AccessFlagsBits
{
    ACCESS_NONE                           = 0x00000000,
    ACCESS_COLOR_ATTACHMENT_READ          = 0x00000001,
    ACCESS_COLOR_ATTACHMENT_WRITE         = 0x00000002,
    ACCESS_DEPTH_STENCIL_ATTACHMENT_READ  = 0x00000003,
    ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE = 0x00000004,
    ACCESS_TRANSFER_READ                  = 0x00000005,
    ACCESS_TRANSFER_WRITE                 = 0x00000006,
    ACCESS_SHADER_READ                    = 0x00000007,
    ACCESS_SHADER_WRITE                   = 0x00000008,
    ACCESS_MAX                            = 0x00000009
} AccessFlags;
typedef enum AttachmentStoreOpFlagsBits
{
    ATTACHMENT_STORE_OP_NONE      = 0x00000000,
    ATTACHMENT_STORE_OP_STORE     = 0x00000001,
    ATTACHMENT_STORE_OP_DONT_CARE = 0x00000002,
} AttachmentStoreOp;
typedef enum AttachmentLoadOpFlagBits
{
    ATTACHMENT_LOAD_OP_LOAD      = 0x00000000,
    ATTACHMENT_LOAD_OP_CLEAR     = 0x00000001,
    ATTACHMENT_LOAD_OP_DONT_CARE = 0x00000002
} AttachmentLoadOp;
typedef enum TextureFormatTypeFlagBits
{
    TEXTURE_FORMAT_TYPE_COLOR  = 0x00000000,
    TEXTURE_FORMAT_TYPE_NORMAL = 0x00000001,
    TEXTURE_FORMAT_TYPE_DEPTH  = 0x00000002,
    TEXTURE_FORMAT_TYPE_HDR    = 0x00000003
} TextureFormatType;
typedef enum BindingTypeFlagBits
{
    BINDING_TYPE_GRAPHIC    = 0x00000000,
    BINDING_TYPE_COMPUTE    = 0x00000001,
    BINDING_TYPE_RAYTRACING = 0x00000002
} BindingType;

typedef enum ImageUsageFlagsBits
{
    USAGE_NONE                     = 0x0,
    USAGE_SAMPLED                  = 0x1,
    USAGE_STORAGE                  = 0x2,
    USAGE_TRANSFER_SRC             = 0x4,
    USAGE_TRANSFER_DST             = 0x8,
    USAGE_COLOR_ATTACHMENT         = 0x10,
    USAGE_DEPTH_STENCIL_ATTACHMENT = 0x20,
    USAGE_TRANSIENT_ATTACHMENT     = 0x40,
    USAGE_INPUT_ATTACHMENT         = 0x80,
} ImageUsageFlags;
inline ImageUsageFlags operator|(ImageUsageFlags a, ImageUsageFlags b) {
    return static_cast<ImageUsageFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline ImageUsageFlags operator&(ImageUsageFlags a, ImageUsageFlags b) {
    return static_cast<ImageUsageFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline ImageUsageFlags& operator|=(ImageUsageFlags& a, ImageUsageFlags b) {
    a = a | b;
    return a;
}
inline ImageUsageFlags& operator&=(ImageUsageFlags& a, ImageUsageFlags b) {
    a = a & b;
    return a;
}
inline ImageUsageFlags operator~(ImageUsageFlags a) {
    return static_cast<ImageUsageFlags>(~static_cast<uint32_t>(a));
}
/*
Graphic pipeline result info
*/
typedef enum RenderResult
{
    SUCCESS                                            = VK_SUCCESS,
    NOT_READY                                          = VK_NOT_READY,
    TIMEOUT                                            = VK_TIMEOUT,
    EVENT_SET                                          = VK_EVENT_SET,
    EVENT_RESET                                        = VK_EVENT_RESET,
    INCOMPLETE                                         = VK_INCOMPLETE,
    ERROR_OUT_OF_HOST_MEMORY                           = VK_ERROR_OUT_OF_HOST_MEMORY,
    ERROR_OUT_OF_DEVICE_MEMORY                         = VK_ERROR_OUT_OF_DEVICE_MEMORY,
    ERROR_INITIALIZATION_FAILED                        = VK_ERROR_INITIALIZATION_FAILED,
    ERROR_DEVICE_LOST                                  = VK_ERROR_DEVICE_LOST,
    ERROR_MEMORY_MAP_FAILED                            = VK_ERROR_MEMORY_MAP_FAILED,
    ERROR_LAYER_NOT_PRESENT                            = VK_ERROR_LAYER_NOT_PRESENT,
    ERROR_EXTENSION_NOT_PRESENT                        = VK_ERROR_EXTENSION_NOT_PRESENT,
    ERROR_FEATURE_NOT_PRESENT                          = VK_ERROR_FEATURE_NOT_PRESENT,
    ERROR_INCOMPATIBLE_DRIVER                          = VK_ERROR_INCOMPATIBLE_DRIVER,
    ERROR_TOO_MANY_OBJECTS                             = VK_ERROR_TOO_MANY_OBJECTS,
    ERROR_FORMAT_NOT_SUPPORTED                         = VK_ERROR_FORMAT_NOT_SUPPORTED,
    ERROR_FRAGMENTED_POOL                              = VK_ERROR_FRAGMENTED_POOL,
    ERROR_UNKNOWN                                      = VK_ERROR_UNKNOWN,
    ERROR_OUT_OF_POOL_MEMORY                           = VK_ERROR_OUT_OF_POOL_MEMORY,
    ERROR_INVALID_EXTERNAL_HANDLE                      = VK_ERROR_INVALID_EXTERNAL_HANDLE,
    ERROR_FRAGMENTATION                                = VK_ERROR_FRAGMENTATION,
    ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS               = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
    PIPELINE_COMPILE_REQUIRED                          = VK_PIPELINE_COMPILE_REQUIRED,
    ERROR_SURFACE_LOST_KHR                             = VK_ERROR_SURFACE_LOST_KHR,
    ERROR_NATIVE_WINDOW_IN_USE_KHR                     = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
    SUBOPTIMAL_KHR                                     = VK_SUBOPTIMAL_KHR,
    ERROR_OUT_OF_DATE_KHR                              = VK_ERROR_OUT_OF_DATE_KHR,
    ERROR_INCOMPATIBLE_DISPLAY_KHR                     = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
    ERROR_VALIDATION_FAILED_EXT                        = VK_ERROR_VALIDATION_FAILED_EXT,
    ERROR_INVALID_SHADER_NV                            = VK_ERROR_INVALID_SHADER_NV,
    ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR                = VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,
    ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR       = VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,
    ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR    = VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,
    ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR       = VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,
    ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR        = VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR,
    ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR          = VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR,
    ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT = VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
    ERROR_NOT_PERMITTED_KHR                            = VK_ERROR_NOT_PERMITTED_KHR,
    ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT          = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
    THREAD_IDLE_KHR                                    = VK_THREAD_IDLE_KHR,
    THREAD_DONE_KHR                                    = VK_THREAD_DONE_KHR,
    OPERATION_DEFERRED_KHR                             = VK_OPERATION_DEFERRED_KHR,
    OPERATION_NOT_DEFERRED_KHR                         = VK_OPERATION_NOT_DEFERRED_KHR,
    ERROR_COMPRESSION_EXHAUSTED_EXT                    = VK_ERROR_COMPRESSION_EXHAUSTED_EXT,
    ERROR_OUT_OF_POOL_MEMORY_KHR                       = VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
    ERROR_INVALID_EXTERNAL_HANDLE_KHR                  = VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
    ERROR_FRAGMENTATION_EXT                            = VK_ERROR_FRAGMENTATION_EXT,
    ERROR_NOT_PERMITTED_EXT                            = VK_ERROR_NOT_PERMITTED_EXT,
    ERROR_INVALID_DEVICE_ADDRESS_EXT                   = VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
    ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR           = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR,
    PIPELINE_COMPILE_REQUIRED_EXT                      = VK_PIPELINE_COMPILE_REQUIRED_EXT,
    ERROR_PIPELINE_COMPILE_REQUIRED_EXT                = VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT,
    ERROR_INCOMPATIBLE_SHADER_BINARY_EXT               = VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT,
    RESULT_MAX_ENUM                                    = VK_RESULT_MAX_ENUM
    // INCOMPATIBLE_SHADER_BINARY_EXT                     = VK_INCOMPATIBLE_SHADER_BINARY_EXT,
    // ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR             = VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR,
} RenderResult;

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

typedef enum GuiColorProfileType
{
    DARK    = 0,
    BRIGHT  = 1,
    CLASSIC = 2,
    CUSTOM  = 3
} GuiColorProfileType;

typedef enum TextWidgetType
{
    SIMPLE,
    COLORIZED,
    WARPED,
    BULLET,
} TextWidgetType;

VULKAN_ENGINE_NAMESPACE_END

#endif
