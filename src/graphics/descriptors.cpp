#include <engine/graphics/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void DescriptorPool::set_layout(uint32_t                         layoutSetIndex,
                                std::vector<LayoutBinding>       bindings,
                                VkDescriptorSetLayoutCreateFlags flags) {

    std::vector<VkDescriptorSetLayoutBinding> bindingHandles;
    bindingHandles.resize(bindings.size());
    for (size_t i = 0; i < bindings.size(); i++)
    {
        bindingHandles[i] = bindings[i].handle;
    }

    VkDescriptorSetLayout           layout;
    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.bindingCount                    = static_cast<uint32_t>(bindingHandles.size());
    setinfo.flags                           = flags;
    setinfo.pNext                           = nullptr;
    setinfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings                       = bindingHandles.data();

    VK_CHECK(vkCreateDescriptorSetLayout(device, &setinfo, nullptr, &layout));

    layouts[layoutSetIndex] = layout;
}
void DescriptorPool::allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor) {
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext                       = nullptr;
    allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool              = handle;
    allocInfo.descriptorSetCount          = 1;
    allocInfo.pSetLayouts                 = &layouts[layoutSetIndex];

    descriptor->layoutID = layoutSetIndex;

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptor->handle));

    descriptor->allocated = true;
}
void DescriptorPool::set_descriptor_write(Buffer*         buffer,
                                          size_t          dataSize,
                                          size_t          readOffset,
                                          DescriptorSet*  descriptor,
                                          UniformDataType type,
                                          uint32_t        binding) {
    VkDescriptorBufferInfo info;
    info.buffer = buffer->handle;
    info.offset = static_cast<VkDeviceSize>(readOffset);
    info.range  = static_cast<VkDeviceSize>(dataSize);

    VkWriteDescriptorSet writeSetting =
        Init::write_descriptor_buffer(Translator::get(type), descriptor->handle, &info, binding);

    descriptor->bindings += 1;
    descriptor->binded_buffers.push_back(buffer);
    descriptor->isDynamic = type == UNIFORM_DYNAMIC_BUFFER;

    vkUpdateDescriptorSets(device, 1, &writeSetting, 0, nullptr);
}
void DescriptorPool::set_descriptor_write(Image*         image,
                                          ImageLayout    layout,
                                          DescriptorSet* descriptor,
                                          uint32_t       binding) {

    VkDescriptorImageInfo imageBufferInfo;
    imageBufferInfo.sampler     = image->sampler;
    imageBufferInfo.imageView   = image->view;
    imageBufferInfo.imageLayout = Translator::get(layout);

    VkWriteDescriptorSet texture1 = Init::write_descriptor_image(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor->handle, &imageBufferInfo, binding);

    descriptor->bindings += 1;

    vkUpdateDescriptorSets(device, 1, &texture1, 0, nullptr);
}
void DescriptorPool::set_descriptor_write(TLAS* accel, DescriptorSet* descriptor, uint32_t binding) {

    VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo =
        Init::write_descriptor_set_acceleration_structure();
    descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
    descriptorAccelerationStructureInfo.pAccelerationStructures    = &accel->handle;

    VkWriteDescriptorSet accelerationStructureWrite{};
    accelerationStructureWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    accelerationStructureWrite.pNext           = &descriptorAccelerationStructureInfo;
    accelerationStructureWrite.dstSet          = descriptor->handle;
    accelerationStructureWrite.dstBinding      = binding;
    accelerationStructureWrite.descriptorCount = 1;
    accelerationStructureWrite.descriptorType  = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    descriptor->bindings += 1;

    vkUpdateDescriptorSets(device, 1, &accelerationStructureWrite, 0, nullptr);
}
void DescriptorPool::cleanup() {

    for (auto& layout : layouts)
    {
        vkDestroyDescriptorSetLayout(device, layout.second, nullptr);
    }
    if (handle)
    {
        vkDestroyDescriptorPool(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END