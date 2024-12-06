#include <engine/graphics/utilities/translator.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {
namespace Translator {

// ColorFormatType -> VkFormat
VkFormat get(ColorFormatType colorFormatType) {
    switch (colorFormatType)
    {
    case ColorFormatType::SR_8:
        return VK_FORMAT_R8_SRGB;
    case ColorFormatType::SRG_8:
        return VK_FORMAT_R8G8_SRGB;
    case ColorFormatType::SRGB_8:
        return VK_FORMAT_R8G8B8_SRGB;
    case ColorFormatType::SRGBA_8:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case ColorFormatType::SBGRA_8:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case ColorFormatType::SRG_16F:
        return VK_FORMAT_R16G16_SFLOAT;
    case ColorFormatType::SRG_32F:
        return VK_FORMAT_R32G32_SFLOAT;
    case ColorFormatType::SRGBA_16F:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ColorFormatType::SRGBA_32F:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ColorFormatType::SRGB_32F:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case ColorFormatType::RGBA_8U:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case ColorFormatType::DEPTH_16F:
        return VK_FORMAT_D16_UNORM;
    case ColorFormatType::DEPTH_32F:
        return VK_FORMAT_D32_SFLOAT;
    default:
        throw std::invalid_argument("VKEngine error: Unknown ColorFormatType");
    }
}

// SyncType -> VkPresentModeKHR
VkPresentModeKHR get(SyncType syncType) {
    switch (syncType)
    {
    case SyncType::NONE:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case SyncType::MAILBOX:
        return VK_PRESENT_MODE_MAILBOX_KHR;
    case SyncType::VERTICAL:
        return VK_PRESENT_MODE_FIFO_KHR;
    case SyncType::RELAXED_VERTICAL:
        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    default:
        throw std::invalid_argument("VKEngine error: Unknown SyncType");
    }
}
// UniformDataType -> VkDescriptorType
VkDescriptorType get(UniformDataType uniformDataType) {
    switch (uniformDataType)
    {
    case UniformDataType::UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case UniformDataType::UNIFORM_DYNAMIC_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case UniformDataType::UNIFORM_COMBINED_IMAGE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case UniformDataType::UNIFORM_ACCELERATION_STRUCTURE:
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    case UniformDataType::UNIFORM_STORAGE_IMAGE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    default:
        throw std::invalid_argument("VKEngine error: Unknown UniformDataType");
    }
}

VkImageLayout get(ImageLayout layoutType) {
    switch (layoutType)
    {
    case ImageLayout::LAYOUT_UNDEFINED:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case ImageLayout::LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ImageLayout::LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ImageLayout::LAYOUT_TRANSFER_SRC_OPTIMAL:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ImageLayout::LAYOUT_TRANSFER_DST_OPTIMAL:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ImageLayout::LAYOUT_PRESENT:
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case ImageLayout::LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ImageLayout::LAYOUT_GENERAL:
        return VK_IMAGE_LAYOUT_GENERAL;
    default:
        throw std::invalid_argument("VKEngine error: Unknown ImageLayoutType");
    }
}

VkImageViewType get(TextureTypeFlagBits viewType) {
    switch (viewType)
    {
    case TextureTypeFlagBits::TEXTURE_1D:
        return VK_IMAGE_VIEW_TYPE_1D;
    case TextureTypeFlagBits::TEXTURE_2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    case TextureTypeFlagBits::TEXTURE_3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    case TextureTypeFlagBits::TEXTURE_CUBE:
        return VK_IMAGE_VIEW_TYPE_CUBE;
    case TextureTypeFlagBits::TEXTURE_1D_ARRAY:
        return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case TextureTypeFlagBits::TEXTURE_2D_ARRAY:
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case TextureTypeFlagBits::TEXTURE_CUBE_ARRAY:
        return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default:
        throw std::invalid_argument("VKEngine error: Unknown ImageViewType");
    }
}

VkFilter get(FilterType filterType) {
    switch (filterType)
    {
    case FilterType::FILTER_NEAREST:
        return VK_FILTER_NEAREST;
    case FilterType::FILTER_LINEAR:
        return VK_FILTER_LINEAR;
    case FilterType::FILTER_CUBIC:
        return VK_FILTER_CUBIC_EXT;
    default:
        throw std::invalid_argument("VKEngine error: Unknown FilterType");
    }
}

VkSamplerAddressMode get(AddressMode addressMode) {
    switch (addressMode)
    {
    case AddressMode::ADDRESS_MODE_REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case AddressMode::ADDRESS_MODE_MIRROR_REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case AddressMode::ADDRESS_MODE_CLAMP_TO_EDGE:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case AddressMode::ADDRESS_MODE_CLAMP_TO_BORDER:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case AddressMode::ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:
        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:
        throw std::invalid_argument("VKEngine error: Unknown AddressMode");
    }
}

VkClearValue get(ClearValueType clearValueType) {
    switch (clearValueType)
    {
    case ClearValueType::CLEAR_COLOR: {
        VkClearValue clearValue = {};
        clearValue.color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
        return clearValue;
    }
    case ClearValueType::CLEAR_DEPTH_STENCIL: {
        VkClearValue clearValue = {};
        clearValue.depthStencil = {1.0f, 0};
        return clearValue;
    }
    default:
        throw std::invalid_argument("VKEngine error: Unknown ClearValueType");
    }
}

VkDependencyFlags get(SubPassDependencyType dependencyFlags) {
    switch (dependencyFlags)
    {
    case SubPassDependencyType::SUBPASS_DEPENDENCY_NONE:
        return 0;
    case SubPassDependencyType::SUBPASS_DEPENDENCY_BY_REGION:
        return VK_DEPENDENCY_BY_REGION_BIT;
    default:
        throw std::invalid_argument("VKEngine error: Unknown SubPassDependencyFlags");
    }
}

VkPipelineStageFlags get(PipelineStage stageFlags) {
    switch (stageFlags)
    {
    case PipelineStage::STAGE_TOP_OF_PIPE:
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case PipelineStage::STAGE_BOTTOM_OF_PIPE:
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case PipelineStage::STAGE_COLOR_ATTACHMENT_OUTPUT:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case PipelineStage::STAGE_EARLY_FRAGMENT_TESTS:
        return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    case PipelineStage::STAGE_LATE_FRAGMENT_TESTS:
        return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case PipelineStage::STAGE_ALL_GRAPHICS:
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    case PipelineStage::STAGE_TRANSFER:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case PipelineStage::STAGE_COMPUTE_SHADER:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case PipelineStage::STAGE_ALL_COMMANDS:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    case PipelineStage::STAGE_FRAGMENT_SHADER:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    default:
        throw std::invalid_argument("VKEngine error: Unknown PipelineStageFlags");
    }
}

VkAccessFlags get(AccessFlags accessFlags) {
    switch (accessFlags)
    {
    case AccessFlags::ACCESS_NONE:
        return 0;
    case AccessFlags::ACCESS_COLOR_ATTACHMENT_READ:
        return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    case AccessFlags::ACCESS_COLOR_ATTACHMENT_WRITE:
        return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case AccessFlags::ACCESS_DEPTH_STENCIL_ATTACHMENT_READ:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case AccessFlags::ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case AccessFlags::ACCESS_TRANSFER_READ:
        return VK_ACCESS_TRANSFER_READ_BIT;
    case AccessFlags::ACCESS_TRANSFER_WRITE:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    case AccessFlags::ACCESS_SHADER_READ:
        return VK_ACCESS_SHADER_READ_BIT;
    case AccessFlags::ACCESS_SHADER_WRITE:
        return VK_ACCESS_SHADER_WRITE_BIT;
    case AccessFlags::ACCESS_MEMORY_READ:
        return VK_ACCESS_MEMORY_READ_BIT;
    default:
        throw std::invalid_argument("VKEngine error: Unknown AccessFlags");
    }
}
VkAttachmentStoreOp get(AttachmentStoreOp storeOp) {
    switch (storeOp)
    {
    case AttachmentStoreOp::ATTACHMENT_STORE_OP_STORE:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case AttachmentStoreOp::ATTACHMENT_STORE_OP_NONE: // Optional, requires Vulkan 1.2 or later.
#ifdef VK_ATTACHMENT_STORE_OP_NONE
        return VK_ATTACHMENT_STORE_OP_NONE;
#else
        throw std::invalid_argument("VK_ATTACHMENT_STORE_OP_NONE not supported in this Vulkan version");
#endif
    default:
        throw std::invalid_argument("VKEngine error: Unknown AttachmentStoreOp");
    }
}

VkAttachmentLoadOp get(AttachmentLoadOp loadOp) {
    switch (loadOp)
    {
    case AttachmentLoadOp::ATTACHMENT_LOAD_OP_LOAD:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case AttachmentLoadOp::ATTACHMENT_LOAD_OP_CLEAR:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case AttachmentLoadOp::ATTACHMENT_LOAD_OP_DONT_CARE:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    default:
        throw std::invalid_argument("VKEngine error: Unknown AttachmentLoadOp");
    }
}
VkImageAspectFlags get(ImageAspect aspectType) {
    switch (aspectType)
    {
    case ImageAspect::ASPECT_COLOR:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    case ImageAspect::ASPECT_DEPTH:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case ImageAspect::ASPECT_STENCIL:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case ImageAspect::ASPECT_METADATA:
        return VK_IMAGE_ASPECT_METADATA_BIT;
    default:
        throw std::invalid_argument("VKEngine error: Unknown Image AspectType");
    }
}
VkSamplerMipmapMode get(MipmapMode mipmapMode) {
    switch (mipmapMode)
    {
    case MipmapMode::MIPMAP_NEAREST:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    case MipmapMode::MIPMAP_LINEAR:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    default:
        throw std::invalid_argument("VKEngine error: Unknown SamplerMipmapMode");
    }
}

VkBorderColor get(BorderColor borderColor) {
    switch (borderColor)
    {
    case BorderColor::FLOAT_TRANSPARENT_BLACK:
        return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case BorderColor::INT_TRANSPARENT_BLACK:
        return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    case BorderColor::FLOAT_OPAQUE_BLACK:
        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case BorderColor::INT_OPAQUE_BLACK:
        return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    case BorderColor::FLOAT_OPAQUE_WHITE:
        return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    case BorderColor::INT_OPAQUE_WHITE:
        return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    default:
        throw std::invalid_argument("VKEngine error: Unknown BorderColor");
    }
}

VkImageUsageFlags get(ImageUsageFlags usageFlags) {
    VkImageUsageFlags vkFlags = 0;

    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_SAMPLED))
    {
        vkFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_STORAGE))
    {
        vkFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_TRANSFER_SRC))
    {
        vkFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_TRANSFER_DST))
    {
        vkFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_COLOR_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) &
        static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_TRANSIENT_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::IMAGE_USAGE_INPUT_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    // Add other flags as needed

    return vkFlags;
}
VkCommandPoolCreateFlags get(CommandPoolCreateFlags flags) {
    switch (flags)
    {
    case CommandPoolCreateFlags::COMMAND_POOL_NONE:
        return 0; // No flags
    case CommandPoolCreateFlags::COMMAND_POOL_CREATE_TRANSIENT:
        return VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    case CommandPoolCreateFlags::COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER:
        return VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    default:
        throw std::invalid_argument("VKEngine error: Unknown CommandPoolCreateFlags");
    }
}
VkCommandBufferLevel get(CommandBufferLevel level) {
    switch (level)
    {
    case CommandBufferLevel::COMMAND_BUFFER_LEVEL_PRIMARY:
        return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    case CommandBufferLevel::COMMAND_BUFFER_LEVEL_SECONDARY:
        return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    default:
        throw std::invalid_argument("VKEngine error: Unknown CommandBufferLevel");
    }
}
VkBufferUsageFlags get(BufferUsageFlags usageFlags) {
    VkBufferUsageFlags vkFlags = 0;

    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_TRANSFER_SRC))
    {
        vkFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_TRANSFER_DST))
    {
        vkFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_UNIFORM_TEXEL_BUFFER))
    {
        vkFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_STORAGE_TEXEL_BUFFER))
    {
        vkFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_UNIFORM_BUFFER))
    {
        vkFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_STORAGE_BUFFER))
    {
        vkFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_INDEX_BUFFER))
    {
        vkFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_VERTEX_BUFFER))
    {
        vkFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_INDIRECT_BUFFER))
    {
        vkFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) &
        static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE))
    {
        vkFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_SHADER_DEVICE_ADDRESS))
    {
        vkFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) &
        static_cast<uint32_t>(BufferUsageFlags::BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY))
    {
        vkFlags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }

    return vkFlags;
}
VkMemoryPropertyFlags get(MemoryPropertyFlags memoryFlags) {
    VkMemoryPropertyFlags vkFlags = 0;

    if (static_cast<uint32_t>(memoryFlags) & static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_DEVICE_LOCAL))
    {
        vkFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    if (static_cast<uint32_t>(memoryFlags) & static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_HOST_VISIBLE))
    {
        vkFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    if (static_cast<uint32_t>(memoryFlags) & static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_HOST_COHERENT))
    {
        vkFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    if (static_cast<uint32_t>(memoryFlags) & static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_HOST_CACHED))
    {
        vkFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }
    if (static_cast<uint32_t>(memoryFlags) &
        static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_LAZILY_ALLOCATED))
    {
        vkFlags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    }
    if (static_cast<uint32_t>(memoryFlags) & static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_PROTECTED))
    {
        vkFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }
    if (static_cast<uint32_t>(memoryFlags) &
        static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_DEVICE_COHERENT))
    {
        vkFlags |= VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
    }
    if (static_cast<uint32_t>(memoryFlags) &
        static_cast<uint32_t>(MemoryPropertyFlags::MEMORY_PROPERTY_DEVICE_UNCACHED))
    {
        vkFlags |= VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
    }

    return vkFlags;
}
VkShaderStageFlags get(ShaderStageFlags shaderStages) {
    VkShaderStageFlags vkFlags = 0;

    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_VERTEX))
    {
        vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (static_cast<uint32_t>(shaderStages) &
        static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_TESSELLATION_CONTROL))
    {
        vkFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    }
    if (static_cast<uint32_t>(shaderStages) &
        static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_TESSELLATION_EVALUATION))
    {
        vkFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_GEOMETRY))
    {
        vkFlags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_FRAGMENT))
    {
        vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_COMPUTE))
    {
        vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_ALL_GRAPHICS))
    {
        vkFlags |= VK_SHADER_STAGE_ALL_GRAPHICS;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_ALL))
    {
        vkFlags |= VK_SHADER_STAGE_ALL;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_RAYGEN))
    {
        vkFlags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_ANY_HIT))
    {
        vkFlags |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_CLOSEST_HIT))
    {
        vkFlags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_MISS))
    {
        vkFlags |= VK_SHADER_STAGE_MISS_BIT_KHR;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_INTERSECTION))
    {
        vkFlags |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_CALLABLE))
    {
        vkFlags |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_TASK))
    {
        vkFlags |= VK_SHADER_STAGE_TASK_BIT_EXT;
    }
    if (static_cast<uint32_t>(shaderStages) & static_cast<uint32_t>(ShaderStageFlags::SHADER_STAGE_MESH))
    {
        vkFlags |= VK_SHADER_STAGE_MESH_BIT_EXT;
    }

    return vkFlags;
}

} // namespace Translator
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END
