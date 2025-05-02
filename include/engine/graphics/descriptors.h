/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H
// HERE YOU WILL ALSO FIND WRAPPER FOR PUSH CONSTANTS

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

    LayoutBinding(UniformDataType type, ShaderStageFlags stageFlags, uint32_t binding, uint32_t descriptorCount = 1U) {
        handle = Init::descriptorset_layout_binding(Translator::get(type), Translator::get(stageFlags), binding, descriptorCount);
    }
};

struct DescriptorPool {
    VkDescriptorPool                                    handle = VK_NULL_HANDLE;
    VkDevice                                            device = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkDescriptorSetLayout> layouts;

    void set_layout(uint32_t                         layoutSetIndex,
                    std::vector<LayoutBinding>       bindings,
                    VkDescriptorSetLayoutCreateFlags flags    = 0,
                    VkDescriptorBindingFlagsEXT      extFlags = 0);

    inline VkDescriptorSetLayout get_layout(uint32_t layoutSetIndex) {
        return layouts[layoutSetIndex];
    }

    void allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor);
    void allocate_variable_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor, uint32_t count);

    /*
    Set writes for Uniform Buffers
    */
    void update_descriptor(Buffer* buffer, size_t dataSize, size_t readOffset, DescriptorSet* descriptor, UniformDataType type, uint32_t binding);
    /*
    Set writes for Images
    */
    void update_descriptor(Image*          image,
                           ImageLayout     layout,
                           DescriptorSet*  descriptor,
                           uint32_t        binding,
                           UniformDataType type = UNIFORM_COMBINED_IMAGE_SAMPLER,
                           uint32_t        arraySlot = 0);
    /*
    Set writes for Image Array
    */
    void update_descriptor(std::vector<Image>& images,
                           ImageLayout         layout,
                           DescriptorSet*      descriptor,
                           uint32_t            binding,
                           UniformDataType     type = UNIFORM_COMBINED_IMAGE_SAMPLER);
    /*
    Set writes for Acceleration Structures
    */
    void update_descriptor(TLAS* accel, DescriptorSet* descriptor, uint32_t binding);

    void cleanup();
};
/*
PUSH CONSTANTS
*/
struct PushConstant {
    VkPushConstantRange handle{};

    PushConstant(ShaderStageFlags stage, uint32_t size, uint32_t offset = 0) {
        handle.stageFlags = Translator::get(stage);
        handle.offset     = offset;
        handle.size       = size;
    }
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif