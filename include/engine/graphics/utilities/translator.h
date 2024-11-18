/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef VULKAN_ENUM_TRANSLATOR_H
#define VULKAN_ENUM_TRANSLATOR_H

#include <engine/common.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
namespace Graphics {
namespace Translator {

VkFilter              get(TextureFilterType filterType);
VkFormat              get(ColorFormatType colorFormatType);
VkSamplerAddressMode  get(TextureAdressModeType addressModeType);
VkPresentModeKHR      get(SyncType syncType);
VkShaderStageFlagBits get(ShaderStageType stageType);
VkDescriptorType      get(UniformDataType uniformDataType);
VkImageUsageFlags     get(ImageUsageFlags usageFlags);
VkImageLayout         get(ImageLayoutType layoutType);
VkImageViewType       get(TextureType viewType);
VkFilter              get(FilterType filterType);
VkSamplerAddressMode  get(AddressMode addressMode);
VkClearValue          get(ClearValueType clearValueType);
VkDependencyFlags     get(SubPassDependencyFlags dependencyFlags);
VkPipelineStageFlags  get(PipelineStageFlags stageFlags);
VkAccessFlags         get(AccessFlags accessFlags);

} // namespace Translator
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END
#endif // VULKAN_ENUM_TRANSLATOR_H
