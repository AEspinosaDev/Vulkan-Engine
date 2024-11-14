/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <engine/common.h>
#include <engine/graphics/accel.h>
#include <engine/graphics/image.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/utilities/translator.h>
#include <unordered_map>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

struct DescriptorSet {
    VkDescriptorSet handle{};

    std::vector<Buffer*> binded_buffers;
    uint32_t             layoutID;
    uint32_t             bindings;
    bool                 isDynamic;
    bool                 allocated{false};
};

struct LayoutBinding {

    VkDescriptorSetLayoutBinding handle{};

    LayoutBinding(UniformDataType    type,
                  VkShaderStageFlags stageFlags,
                  uint32_t           binding,
                  uint32_t           descriptorCount = 1U) {
        handle = Init::descriptorset_layout_binding(Translator::get(type), stageFlags, binding, descriptorCount);
    }
};

class DescriptorPool
{
    VkDescriptorPool                                    m_handle = VK_NULL_HANDLE;
    VkDevice                                            m_device;
    std::unordered_map<uint32_t, VkDescriptorSetLayout> m_layouts;

  public:
    inline VkDescriptorPool& get_handle() {
        return m_handle;
    }
    void init(VkDevice                       dvc,
              uint32_t                       maxSets,
              uint32_t                       numUBO,
              uint32_t                       numUBODynamic,
              uint32_t                       numUBOStorage,
              uint32_t                       numImageCombined,
              uint32_t                       numSampler           = 0,
              uint32_t                       numSampledImage      = 0,
              uint32_t                       numStrgImage         = 0,
              uint32_t                       numUBTexel           = 0,
              uint32_t                       numStrgTexel         = 0,
              uint32_t                       numUBOStorageDynamic = 0,
              uint32_t                       numIAttachment       = 0,
              VkDescriptorPoolCreateFlagBits flag                 = {});

    void set_layout(uint32_t                         layoutSetIndex,
                    std::vector<LayoutBinding>       bindings,
                    VkDescriptorSetLayoutCreateFlags flags = 0);

    inline VkDescriptorSetLayout get_layout(uint32_t layoutSetIndex) {
        return m_layouts[layoutSetIndex];
    }

    void allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor);

    /*
    Set writes for Uniform Buffers
    */
    void set_descriptor_write(Buffer*          buffer,
                              VkDeviceSize     dataSize,
                              VkDeviceSize     readOffset,
                              DescriptorSet*   descriptor,
                              VkDescriptorType type,
                              uint32_t         binding);

    /*
    Set writes for Images
    */
    void set_descriptor_write(Image* image, VkImageLayout layout, DescriptorSet* descriptor, uint32_t binding);
    /*
    Set writes for Acceleration Structures
    */
    void set_descriptor_write(TLAS* accel, DescriptorSet* descriptor, uint32_t binding);

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif