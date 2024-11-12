#include <engine/graphics/utilities/translator.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {
namespace Translator {

VkFilter get(TextureFilterType filterType) {
    switch (filterType)
    {
    case TextureFilterType::NEAREST:
        return VK_FILTER_NEAREST;
    case TextureFilterType::LINEAR:
        return VK_FILTER_LINEAR;
    case TextureFilterType::CUBIC:
        return VK_FILTER_CUBIC_EXT;
    case TextureFilterType::MAX:
        return VK_FILTER_MAX_ENUM;
    default:
        throw std::invalid_argument("Unknown TextureFilterType");
    }
}

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
    default:
        throw std::invalid_argument("Unknown ColorFormatType");
    }
}

// DepthFormatType -> VkFormat
VkFormat get(DepthFormatType depthFormatType) {
    switch (depthFormatType)
    {
    case DepthFormatType::D16F:
        return VK_FORMAT_D16_UNORM;
    case DepthFormatType::D32F:
        return VK_FORMAT_D32_SFLOAT;
    default:
        throw std::invalid_argument("Unknown DepthFormatType");
    }
}

// TextureAdressModeType -> VkSamplerAddressMode
VkSamplerAddressMode get(TextureAdressModeType addressModeType) {
    switch (addressModeType)
    {
    case TextureAdressModeType::REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureAdressModeType::MIRROR_REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureAdressModeType::EDGE_CLAMP:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureAdressModeType::MIRROR_EDGE_CLAMP:
        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    case TextureAdressModeType::BORDER_CLAMP:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    default:
        throw std::invalid_argument("Unknown TextureAdressModeType");
    }
}

// SyncType -> VkPresentModeKHR
VkPresentModeKHR get(SyncType syncType) {
    switch (syncType)
    {
    case SyncType::NONE_SYNC:
        return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case SyncType::MAILBOX_SYNC:
        return VK_PRESENT_MODE_MAILBOX_KHR;
    case SyncType::V_SYNC:
        return VK_PRESENT_MODE_FIFO_KHR;
    case SyncType::RELAXED_V_SYNC:
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
    default:
        throw std::invalid_argument("Unknown UniformDataType");
    }
}

VkSampleCountFlagBits get(SampleCount sampleCount) {
    switch (sampleCount)
    {
    case SampleCount::SAMPLE_COUNT_1:
        return VK_SAMPLE_COUNT_1_BIT;
    case SampleCount::SAMPLE_COUNT_2:
        return VK_SAMPLE_COUNT_2_BIT;
    case SampleCount::SAMPLE_COUNT_4:
        return VK_SAMPLE_COUNT_4_BIT;
    case SampleCount::SAMPLE_COUNT_8:
        return VK_SAMPLE_COUNT_8_BIT;
    case SampleCount::SAMPLE_COUNT_16:
        return VK_SAMPLE_COUNT_16_BIT;
    case SampleCount::SAMPLE_COUNT_32:
        return VK_SAMPLE_COUNT_32_BIT;
    case SampleCount::SAMPLE_COUNT_64:
        return VK_SAMPLE_COUNT_64_BIT;
    default:
        throw std::invalid_argument("Unknown SampleCount");
    }
}

VkImageUsageFlags get(ImageUsageFlags usageFlags) {
    switch (usageFlags)
    {
    case ImageUsageFlags::COLOR_ATTACHMENT:
        return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    case ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT:
        return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    case ImageUsageFlags::SAMPLED:
        return VK_IMAGE_USAGE_SAMPLED_BIT;
    case ImageUsageFlags::STORAGE:
        return VK_IMAGE_USAGE_STORAGE_BIT;
    case ImageUsageFlags::TRANSFER_SRC:
        return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    case ImageUsageFlags::TRANSFER_DST:
        return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    case ImageUsageFlags::SHADER_READ_ONLY:
        return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    case ImageUsageFlags::TRANSFER:
        return VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    default:
        throw std::invalid_argument("Unknown ImageUsageFlags");
    }
}

VkImageLayout get(ImageLayoutType layoutType) {
    switch (layoutType)
    {
    case ImageLayoutType::UNDEFINED:
        return VK_IMAGE_LAYOUT_UNDEFINED;
    case ImageLayoutType::COLOR_ATTACHMENT_OPTIMAL:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ImageLayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ImageLayoutType::SHADER_READ_ONLY_OPTIMAL:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ImageLayoutType::TRANSFER_SRC_OPTIMAL:
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ImageLayoutType::TRANSFER_DST_OPTIMAL:
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    default:
        throw std::invalid_argument("Unknown ImageLayoutType");
    }
}

VkImageViewType get(ImageViewType viewType) {
    switch (viewType)
    {
    case ImageViewType::TYPE_1D:
        return VK_IMAGE_VIEW_TYPE_1D;
    case ImageViewType::TYPE_2D:
        return VK_IMAGE_VIEW_TYPE_2D;
    case ImageViewType::TYPE_3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    case ImageViewType::TYPE_CUBE:
        return VK_IMAGE_VIEW_TYPE_CUBE;
    case ImageViewType::TYPE_1D_ARRAY:
        return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case ImageViewType::TYPE_2D_ARRAY:
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case ImageViewType::TYPE_CUBE_ARRAY:
        return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default:
        throw std::invalid_argument("Unknown ImageViewType");
    }
}

VkFilter get(FilterType filterType) {
    switch (filterType)
    {
    case FilterType::NEAREST:
        return VK_FILTER_NEAREST;
    case FilterType::LINEAR:
        return VK_FILTER_LINEAR;
    case FilterType::CUBIC:
        return VK_FILTER_CUBIC_EXT;
    default:
        throw std::invalid_argument("Unknown FilterType");
    }
}

VkSamplerAddressMode get(AddressMode addressMode) {
    switch (addressMode)
    {
    case AddressMode::REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case AddressMode::MIRROR_REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case AddressMode::CLAMP_TO_EDGE:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case AddressMode::CLAMP_TO_BORDER:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case AddressMode::MIRROR_CLAMP_TO_EDGE:
        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:
        throw std::invalid_argument("Unknown AddressMode");
    }
}

VkClearValue get(ClearValueType clearValueType) {
    switch (clearValueType)
    {
    case ClearValueType::COLOR: {
        VkClearValue clearValue = {};
        clearValue.color        = {{0.0f, 0.0f, 0.0f, 1.0f}};
        return clearValue;
    }
    case ClearValueType::DEPTH_STENCIL: {
        VkClearValue clearValue = {};
        clearValue.depthStencil = {1.0f, 0};
        return clearValue;
    }
    default:
        throw std::invalid_argument("Unknown ClearValueType");
    }
}

VkDependencyFlags get(SubPassDependencyFlags dependencyFlags) {
    switch (dependencyFlags)
    {
    case SubPassDependencyFlags::BY_REGION:
        return VK_DEPENDENCY_BY_REGION_BIT;
    default:
        throw std::invalid_argument("Unknown SubPassDependencyFlags");
    }
}

VkPipelineStageFlags get(PipelineStageFlags stageFlags) {
    switch (stageFlags)
    {
    case PipelineStageFlags::TOP_OF_PIPE:
        return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    case PipelineStageFlags::BOTTOM_OF_PIPE:
        return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    case PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT:
        return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    case PipelineStageFlags::EARLY_FRAGMENT_TESTS:
        return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    case PipelineStageFlags::LATE_FRAGMENT_TESTS:
        return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    case PipelineStageFlags::ALL_GRAPHICS:
        return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    case PipelineStageFlags::TRANSFER:
        return VK_PIPELINE_STAGE_TRANSFER_BIT;
    case PipelineStageFlags::COMPUTE_SHADER:
        return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    case PipelineStageFlags::ALL_COMMANDS:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    default:
        throw std::invalid_argument("Unknown PipelineStageFlags");
    }
}

VkAccessFlags get(AccessFlags accessFlags) {
    switch (accessFlags)
    {
    case AccessFlags::ACCESS_NONE:
        return 0;
    case AccessFlags::COLOR_ATTACHMENT_READ:
        return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    case AccessFlags::COLOR_ATTACHMENT_WRITE:
        return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    case AccessFlags::DEPTH_STENCIL_ATTACHMENT_READ:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE:
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    case AccessFlags::TRANSFER_READ:
        return VK_ACCESS_TRANSFER_READ_BIT;
    case AccessFlags::TRANSFER_WRITE:
        return VK_ACCESS_TRANSFER_WRITE_BIT;
    case AccessFlags::SHADER_READ:
        return VK_ACCESS_SHADER_READ_BIT;
    case AccessFlags::SHADER_WRITE:
        return VK_ACCESS_SHADER_WRITE_BIT;
    default:
        throw std::invalid_argument("Unknown AccessFlags");
    }
}
} // namespace Translator
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END
