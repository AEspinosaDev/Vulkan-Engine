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
    case ColorFormatType::RGBA_8U:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case ColorFormatType::DEPTH_16F:
        return VK_FORMAT_D16_UNORM;
    case ColorFormatType::DEPTH_32F:
        return VK_FORMAT_D32_SFLOAT;
    default:
        throw std::invalid_argument("Unknown ColorFormatType");
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
        throw std::invalid_argument("Unknown SyncType");
    }
}

// ShaderStageType -> VkShaderStageFlagBits
VkShaderStageFlagBits get(ShaderStageType stageType) {
    switch (stageType)
    {
    case ShaderStageType::VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderStageType::FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case ShaderStageType::GEOMETRY:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    case ShaderStageType::TESS_CONTROL:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    case ShaderStageType::TESS_EVALUATION:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    case ShaderStageType::NONE_STAGE:
        return static_cast<VkShaderStageFlagBits>(0);
    // case ShaderStageType::ALL_STAGES:
    //     return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    default:
        throw std::invalid_argument("Unknown ShaderStageType");
    }
}

// UniformDataType -> VkDescriptorType
VkDescriptorType get(UniformDataType uniformDataType) {
    switch (uniformDataType)
    {
    case UniformDataType::UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case UniformDataType::DYNAMIC_UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case UniformDataType::COMBINED_IMAGE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case UniformDataType::ACCELERATION_STRUCTURE:
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    default:
        throw std::invalid_argument("Unknown UniformDataType");
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
    default:
        throw std::invalid_argument("Unknown ImageLayoutType");
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
        throw std::invalid_argument("Unknown ImageViewType");
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
        throw std::invalid_argument("Unknown FilterType");
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
        throw std::invalid_argument("Unknown AddressMode");
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
        throw std::invalid_argument("Unknown ClearValueType");
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
        throw std::invalid_argument("Unknown SubPassDependencyFlags");
    }
}

VkPipelineStageFlags get(PipelineStage stageFlags) {
    switch (stageFlags)
    {
    case PipelineStage::TOP_OF_PIPE_STAGE:
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case PipelineStage::BOTTOM_OF_PIPE_STAGE:
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case PipelineStage::COLOR_ATTACHMENT_OUTPUT_STAGE:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case PipelineStage::EARLY_FRAGMENT_TESTS_STAGE:
        return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    case PipelineStage::LATE_FRAGMENT_TESTS_STAGE:
        return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case PipelineStage::ALL_GRAPHICS_STAGE:
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    case PipelineStage::TRANSFER_STAGE:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case PipelineStage::COMPUTE_SHADER_STAGE:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case PipelineStage::ALL_COMMANDS_STAGE:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    case PipelineStage::FRAGMENT_SHADER_STAGE:
        return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    default:
        throw std::invalid_argument("Unknown PipelineStageFlags");
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
    default:
        throw std::invalid_argument("Unknown AccessFlags");
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
        throw std::invalid_argument("Unknown AttachmentStoreOp");
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
        throw std::invalid_argument("Unknown AttachmentLoadOp");
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
        throw std::invalid_argument("Unknown Image AspectType");
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
        throw std::invalid_argument("Unknown SamplerMipmapMode");
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
        throw std::invalid_argument("Unknown BorderColor");
    }
}

VkImageUsageFlags get(ImageUsageFlags usageFlags) {
    VkImageUsageFlags vkFlags = 0;

    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_SAMPLED))
    {
        vkFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_STORAGE))
    {
        vkFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_TRANSFER_SRC))
    {
        vkFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_TRANSFER_DST))
    {
        vkFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_COLOR_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_DEPTH_STENCIL_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_TRANSIENT_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(ImageUsageFlags::USAGE_INPUT_ATTACHMENT))
    {
        vkFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    // Add other flags as needed

    return vkFlags;
}


} // namespace Translator
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END
