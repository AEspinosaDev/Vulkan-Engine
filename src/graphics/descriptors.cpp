#include <engine/graphics/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

    void DescriptorSet::update(Buffer* buffer, size_t dataSize, size_t readOffset, UniformDataType type, uint32_t binding) {
        VkBuffer newBuffer = buffer->handle;
    
        auto it = boundSlots.find(binding);
        if (it != boundSlots.end() && std::holds_alternative<VkBuffer>(it->second) && std::get<VkBuffer>(it->second) == newBuffer)
        {
            // Buffer is already bound — skip update
            return;
        }
    
        VkDescriptorBufferInfo info = {};
        info.buffer                 = newBuffer;
        info.offset                 = static_cast<VkDeviceSize>(readOffset);
        info.range                  = static_cast<VkDeviceSize>(dataSize);
    
        VkWriteDescriptorSet writeSetting = Init::write_descriptor_buffer(Translator::get(type), handle, &info, binding);
    
        vkUpdateDescriptorSets(device, 1, &writeSetting, 0, nullptr);
    
        // Track resource
        boundSlots[binding] = newBuffer;
    }
    void DescriptorSet::update(Image*          image,
                                           ImageLayout     layout,
                                           uint32_t        binding,
                                           UniformDataType type,
                                           uint32_t        arraySlot) {
        VkImageView newView = image->view;
    
        if (!isArrayed) // If standard descriptor set (not variant)
        {
            auto it = boundSlots.find(binding);
            if (it != boundSlots.end() && std::holds_alternative<VkImageView>(it->second) && std::get<VkImageView>(it->second) == newView)
            {
                // Image is already bound — skip update
                return;
            } else
            {
                // Track resource
                boundSlots[binding] = image->view;
            }
        } else
        {
    
            auto it = boundArraySlots.find(arraySlot);
            if (it != boundArraySlots.end() && std::holds_alternative<VkImageView>(it->second) && std::get<VkImageView>(it->second) == newView)
            {
                // Image in array is already bound — skip update
                return;
            } else
            {
                // Track resource
                boundArraySlots[arraySlot] = image->view;
            }
        }
    
        VkDescriptorImageInfo imageBufferInfo;
        imageBufferInfo.sampler     = image->sampler;
        imageBufferInfo.imageView   = image->view;
        imageBufferInfo.imageLayout = Translator::get(layout);
    
        VkWriteDescriptorSet texture1 = Init::write_descriptor_image(Translator::get(type), handle, &imageBufferInfo, 1, arraySlot, binding);
    
        vkUpdateDescriptorSets(device, 1, &texture1, 0, nullptr);
    }
    void DescriptorSet::update(const Image          &image,
                                           ImageLayout     layout,
                                           uint32_t        binding,
                                           UniformDataType type,
                                           uint32_t        arraySlot) {
        VkImageView newView = image.view;
    
        if (!isArrayed) // If standard descriptor set (not variant)
        {
            auto it = boundSlots.find(binding);
            if (it != boundSlots.end() && std::holds_alternative<VkImageView>(it->second) && std::get<VkImageView>(it->second) == newView)
            {
                // Image is already bound — skip update
                return;
            } else
            {
                // Track resource
                boundSlots[binding] = image.view;
            }
        } else
        {
    
            auto it = boundArraySlots.find(arraySlot);
            if (it != boundArraySlots.end() && std::holds_alternative<VkImageView>(it->second) && std::get<VkImageView>(it->second) == newView)
            {
                // Image in array is already bound — skip update
                return;
            } else
            {
                // Track resource
                boundArraySlots[arraySlot] = image.view;
            }
        }
    
        VkDescriptorImageInfo imageBufferInfo;
        imageBufferInfo.sampler     = image.sampler;
        imageBufferInfo.imageView   = image.view;
        imageBufferInfo.imageLayout = Translator::get(layout);
    
        VkWriteDescriptorSet texture1 = Init::write_descriptor_image(Translator::get(type), handle, &imageBufferInfo, 1, arraySlot, binding);
    
        vkUpdateDescriptorSets(device, 1, &texture1, 0, nullptr);
    }
    void DescriptorSet::update(std::vector<Image>& images, ImageLayout layout, uint32_t binding, UniformDataType type) {
    
        VkImageView newView = images[0].view;
    
        auto it = boundSlots.find(binding);
        if (it != boundSlots.end() && std::holds_alternative<VkImageView>(it->second) && std::get<VkImageView>(it->second) == newView)
        {
            // Image is already bound — skip update
            return;
        }
    
        std::vector<VkDescriptorImageInfo> descriptorImageInfos(images.size());
        for (size_t i = 0; i < images.size(); i++)
        {
            descriptorImageInfos[i].sampler     = images[i].sampler;
            descriptorImageInfos[i].imageView   = images[i].view;
            descriptorImageInfos[i].imageLayout = Translator::get(layout);
        }
    
        VkWriteDescriptorSet imageArray =
            Init::write_descriptor_image(Translator::get(type), handle, descriptorImageInfos.data(), images.size(), 0, binding);
    
        vkUpdateDescriptorSets(device, 1, &imageArray, 0, nullptr);
    
        // Track resource
        boundSlots[binding] = newView;
    }
    void DescriptorSet::update(TLAS* accel, uint32_t binding) {
    
        VkAccelerationStructureKHR newAS = accel->handle;
    
        auto it = boundSlots.find(binding);
        if (it != boundSlots.end() && std::holds_alternative<VkAccelerationStructureKHR>(it->second) &&
            std::get<VkAccelerationStructureKHR>(it->second) == newAS)
        {
            // AS is already bound — skip update
            return;
        }
    
        VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo = Init::write_descriptor_set_acceleration_structure();
        descriptorAccelerationStructureInfo.accelerationStructureCount                   = 1;
        descriptorAccelerationStructureInfo.pAccelerationStructures                      = &accel->handle;
    
        VkWriteDescriptorSet accelerationStructureWrite{};
        accelerationStructureWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        accelerationStructureWrite.pNext           = &descriptorAccelerationStructureInfo;
        accelerationStructureWrite.dstSet          = handle;
        accelerationStructureWrite.dstBinding      = binding;
        accelerationStructureWrite.descriptorCount = 1;
        accelerationStructureWrite.descriptorType  = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    
        vkUpdateDescriptorSets(device, 1, &accelerationStructureWrite, 0, nullptr);
    
        // Track resource
        boundSlots[binding] = accel->handle;
    }

void DescriptorPool::set_layout(uint32_t                         layoutSetIndex,
                                std::vector<LayoutBinding>       bindings,
                                VkDescriptorSetLayoutCreateFlags flags,
                                VkDescriptorBindingFlagsEXT      extFlags) {

    std::vector<VkDescriptorSetLayoutBinding> bindingHandles;
    bindingHandles.resize(bindings.size());
    for (size_t i = 0; i < bindings.size(); i++)
    {
        bindingHandles[i] = bindings[i].handle;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlagsInfo = {};
    bindingFlagsInfo.sType                                          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    bindingFlagsInfo.bindingCount                                   = static_cast<uint32_t>(bindingHandles.size());
    bindingFlagsInfo.pBindingFlags                                  = &extFlags;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.bindingCount                    = static_cast<uint32_t>(bindingHandles.size());
    setinfo.flags                           = flags;
    setinfo.pNext                           = extFlags != 0 ? &bindingFlagsInfo : nullptr;
    setinfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings                       = bindingHandles.data();

    DescriptorLayout layout{};
    VK_CHECK(vkCreateDescriptorSetLayout(device, &setinfo, nullptr, &layout.handle));
    layout.device   = device;
    layout.bindings = bindings;

    layouts[layoutSetIndex] = layout;
}
void DescriptorPool::allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor) {

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext                       = nullptr;
    allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool              = handle;
    allocInfo.descriptorSetCount          = 1;
    allocInfo.pSetLayouts                 = &layouts[layoutSetIndex].handle;

    descriptor->layoutID = layoutSetIndex;
    descriptor->device = device;
    
    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptor->handle));
}
void DescriptorPool::allocate_variable_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor, uint32_t count) {
    
    VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo = {};
    countInfo.sType                                              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    countInfo.descriptorSetCount                                 = 1;
    countInfo.pDescriptorCounts                                  = &count;
    
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext                       = &countInfo;
    allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool              = handle;
    allocInfo.descriptorSetCount          = 1;
    allocInfo.pSetLayouts                 = &layouts[layoutSetIndex].handle;
    
    descriptor->layoutID = layoutSetIndex;
    descriptor->device = device;

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptor->handle));

    descriptor->isArrayed = true;
}

void DescriptorPool::cleanup() {

    for (auto& layout : layouts)
    {
        layout.second.cleanup();
    }
    if (handle)
    {
        vkDestroyDescriptorPool(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}
void DescriptorLayout::cleanup() {

    if (handle)
    {
        vkDestroyDescriptorSetLayout(device, handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END