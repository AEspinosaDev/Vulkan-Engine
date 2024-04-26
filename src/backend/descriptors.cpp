#include <engine/backend/descriptors.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void DescriptorManager::create_pool(uint32_t numUBO, uint32_t numUBODynamic, uint32_t numUBOStorage, uint32_t numImageCombined, uint32_t maxSets)
{
    std::vector<VkDescriptorPoolSize> sizes =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numUBO},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numUBODynamic},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numUBOStorage},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numImageCombined}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = maxSets;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    VK_CHECK(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_pool));
}
void DescriptorManager::set_layout(uint32_t layoutSetIndex, VkDescriptorSetLayoutBinding *bindings, uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags)
{
    VkDescriptorSetLayout layout;
    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.bindingCount = bindingCount;
    setinfo.flags = flags;
    setinfo.pNext = nullptr;
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pBindings = bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &setinfo, nullptr, &layout));

    m_layouts[layoutSetIndex] = layout;
}
void DescriptorManager::allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet *descriptor)
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_layouts[layoutSetIndex];

    descriptor->layoutID = layoutSetIndex;

    VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo,
                                      &descriptor->descriptorSet));

    descriptor->allocated = true;
}
void DescriptorManager::set_descriptor_write(Buffer *buffer, VkDeviceSize dataSize, VkDeviceSize readOffset, DescriptorSet *descriptor, VkDescriptorType type, uint32_t binding)
{
    VkDescriptorBufferInfo info;
    info.buffer = buffer->buffer;
    info.offset = readOffset;
    info.range = dataSize;

    VkWriteDescriptorSet writeSetting = init::write_descriptor_buffer(type, descriptor->descriptorSet, &info, binding);

    descriptor->bindings += 1;
    descriptor->binded_buffers.push_back(buffer);
    descriptor->isDynamic = type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    vkUpdateDescriptorSets(m_device, 1, &writeSetting, 0, nullptr);
}
void DescriptorManager::set_descriptor_write(VkSampler sampler, VkImageView imageView, VkImageLayout layout, DescriptorSet *descriptor, uint32_t binding)
{

    VkDescriptorImageInfo imageBufferInfo;
    imageBufferInfo.sampler = sampler;
    imageBufferInfo.imageView = imageView;
    imageBufferInfo.imageLayout = layout;

    VkWriteDescriptorSet texture1 = init::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptor->descriptorSet, &imageBufferInfo, binding);

    descriptor->bindings += 1;

    vkUpdateDescriptorSets(m_device, 1, &texture1, 0, nullptr);
}
void DescriptorManager::cleanup()
{
    for (auto &layout : m_layouts)
    {
        vkDestroyDescriptorSetLayout(m_device, layout.second, nullptr);
    }
    vkDestroyDescriptorPool(m_device, m_pool, nullptr);
}

void DescriptorManager::bind_descriptor_sets(VkCommandBuffer commandBuffer,
                                             VkPipelineBindPoint pipelineBindPoint,
                                             VkPipelineLayout pipelineLayout,
                                             uint32_t firstSet,
                                             const std::vector<DescriptorSet> descriptorSets)
{
    // std::vector<uint32_t> offsets;

    // for (DescriptorSet &descriptor : descriptorSets)
    // {
    //     // TO DO
    // }

    // vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, pipelineLayout, firstSet, descriptorSets.size(), descriptorSets.data(), 3, descriptorOffsets);
}
void DescriptorSet::bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout pipelineLayout, uint32_t firstSet, const std::vector<uint32_t> offsets)
{
    // for
    // vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, pipelineLayout, firstSet,1, descriptorSet, isDynamic? bind: 0, offsets.data());
}

VULKAN_ENGINE_NAMESPACE_END