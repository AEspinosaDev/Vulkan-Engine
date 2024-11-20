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

// Translates engine flags to Vulkan types
namespace Translator {

VkFormat                 get(ColorFormatType colorFormatType);
VkPresentModeKHR         get(SyncType syncType);
VkDescriptorType         get(UniformDataType uniformDataType);
VkImageUsageFlags        get(ImageUsageFlags usageFlags);
VkImageLayout            get(ImageLayout layoutType);
VkImageViewType          get(TextureTypeFlagBits viewType);
VkFilter                 get(FilterType filterType);
VkSamplerAddressMode     get(AddressMode addressMode);
VkClearValue             get(ClearValueType clearValueType);
VkDependencyFlags        get(SubPassDependencyType dependencyFlags);
VkPipelineStageFlags     get(PipelineStage stageFlags);
VkAccessFlags            get(AccessFlags accessFlags);
VkAttachmentStoreOp      get(AttachmentStoreOp storeOp);
VkAttachmentLoadOp       get(AttachmentLoadOp loadOp);
VkImageAspectFlags       get(ImageAspect aspectType);
VkSamplerMipmapMode      get(MipmapMode mipmapMode);
VkBorderColor            get(BorderColor borderColor);
VkCommandPoolCreateFlags get(CommandPoolCreateFlags flags);
VkCommandBufferLevel     get(CommandBufferLevel level);
VkBufferUsageFlags       get(BufferUsageFlags usageFlags);
VkMemoryPropertyFlags    get(MemoryPropertyFlags memoryFlags);
VkShaderStageFlags       get(ShaderStageFlags shaderStages);

} // namespace Translator
} // namespace Graphics
VULKAN_ENGINE_NAMESPACE_END
#endif // VULKAN_ENUM_TRANSLATOR_H
