/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <engine/common.h>
#include <engine/graphics/buffer.h>
#include <engine/graphics/initializers.h>
#include <unordered_map>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

struct DescriptorSet
{
    VkDescriptorSet handle{};

    std::vector<Buffer *> binded_buffers;
    uint32_t layoutID;
    uint32_t bindings;
    bool isDynamic;
    bool allocated{false};

    void bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout pipelineLayout,
              uint32_t firstSet, const std::vector<uint32_t> offsets);
};

class DescriptorManager
{
    VkDevice m_device;
    VkDescriptorPool m_pool{};
    std::unordered_map<uint32_t, VkDescriptorSetLayout> m_layouts;

  public:
    inline void init(VkDevice &dvc)
    {
        m_device = dvc;
    }

    void create_pool(uint32_t numUBO, uint32_t numUBODynamic, uint32_t numUBOStorage, uint32_t numImageCombined,
                     uint32_t maxSets);

    void set_layout(uint32_t layoutSetIndex, VkDescriptorSetLayoutBinding *bindings, uint32_t bindingCount,
                    VkDescriptorSetLayoutCreateFlags flags = 0);

    inline VkDescriptorSetLayout get_layout(uint32_t layoutSetIndex)
    {
        return m_layouts[layoutSetIndex];
    }

    void allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet *descriptor);

    void set_descriptor_write(Buffer *buffer, VkDeviceSize dataSize, VkDeviceSize readOffset, DescriptorSet *descriptor,
                              VkDescriptorType type, uint32_t binding);

    void set_descriptor_write(VkSampler sampler, VkImageView imageView, VkImageLayout layout, DescriptorSet *descriptor,
                              uint32_t binding);

    void bind_descriptor_sets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                              VkPipelineLayout pipelineLayout, uint32_t firstSet,
                              const std::vector<DescriptorSet> descriptorSets);

    void cleanup();
};

} // namespace render

VULKAN_ENGINE_NAMESPACE_END

#endif