#include <engine/graphics/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

void DescriptorPool::init(VkDevice                       dvc,
                          uint32_t                       maxSets,
                          uint32_t                       numUBO,
                          uint32_t                       numUBODynamic,
                          uint32_t                       numUBOStorage,
                          uint32_t                       numImageCombined,
                          uint32_t                       numSampler,
                          uint32_t                       numSampledImage,
                          uint32_t                       numStrgImage,
                          uint32_t                       numUBTexel,
                          uint32_t                       numStrgTexel,
                          uint32_t                       numUBOStorageDynamic,
                          uint32_t                       numIAttachment,
                          VkDescriptorPoolCreateFlagBits flag) {
    m_device = dvc;

    std::vector<VkDescriptorPoolSize> sizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numUBO},
                                               {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numUBODynamic},
                                               {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numUBOStorage},
                                               {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numImageCombined}};
    if (numSampler > 0)
        sizes.push_back({VK_DESCRIPTOR_TYPE_SAMPLER, numSampler});
    if (numSampledImage > 0)
        sizes.push_back({VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, numSampledImage});
    if (numStrgImage > 0)
        sizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, numStrgImage});
    if (numUBTexel > 0)
        sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, numUBTexel});
    if (numStrgTexel > 0)
        sizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, numStrgTexel});
    if (numUBOStorageDynamic > 0)
        sizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, numUBOStorageDynamic});
    if (numIAttachment > 0)
        sizes.push_back({VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, numIAttachment});

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags                      = flag;
    pool_info.maxSets                    = maxSets;
    pool_info.poolSizeCount              = (uint32_t)sizes.size();
    pool_info.pPoolSizes                 = sizes.data();

    VK_CHECK(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_handle));
}
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

    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &setinfo, nullptr, &layout));

    m_layouts[layoutSetIndex] = layout;
}
void DescriptorPool::allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor) {
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext                       = nullptr;
    allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool              = m_handle;
    allocInfo.descriptorSetCount          = 1;
    allocInfo.pSetLayouts                 = &m_layouts[layoutSetIndex];

    descriptor->layoutID = layoutSetIndex;

    VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo, &descriptor->handle));

    descriptor->allocated = true;
}
void DescriptorPool::set_descriptor_write(Buffer*          buffer,
                                          VkDeviceSize     dataSize,
                                          VkDeviceSize     readOffset,
                                          DescriptorSet*   descriptor,
                                          VkDescriptorType type,
                                          uint32_t         binding) {
    VkDescriptorBufferInfo info;
    info.buffer = buffer->handle;
    info.offset = readOffset;
    info.range  = dataSize;

    VkWriteDescriptorSet writeSetting = Init::write_descriptor_buffer(type, descriptor->handle, &info, binding);

    descriptor->bindings += 1;
    descriptor->binded_buffers.push_back(buffer);
    descriptor->isDynamic = type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    vkUpdateDescriptorSets(m_device, 1, &writeSetting, 0, nullptr);
}
void DescriptorPool::set_descriptor_write(Image*         image,
                                          VkImageLayout  layout,
                                          DescriptorSet* descriptor,
                                          uint32_t       binding) {

    VkDescriptorImageInfo imageBufferInfo;
    imageBufferInfo.sampler     = image->sampler;
    imageBufferInfo.imageView   = image->view;
    imageBufferInfo.imageLayout = layout;

    VkWriteDescriptorSet texture1 = Init::write_descriptor_image(
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor->handle, &imageBufferInfo, binding);

    descriptor->bindings += 1;

    vkUpdateDescriptorSets(m_device, 1, &texture1, 0, nullptr);
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

    vkUpdateDescriptorSets(m_device, 1, &accelerationStructureWrite, 0, nullptr);
}
void DescriptorPool::cleanup() {

    for (auto& layout : m_layouts)
    {
        vkDestroyDescriptorSetLayout(m_device, layout.second, nullptr);
    }
    vkDestroyDescriptorPool(m_device, m_handle, nullptr);
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END