#include "vk_descriptors.h"

namespace vke
{

    void DescriptorManager::create_pool(uint32_t numUBO, uint32_t numUBODynamic, uint32_t numUBOStorage, uint32_t maxSets)
    {
        std::vector<VkDescriptorPoolSize> sizes =
            {
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numUBO},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numUBODynamic},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numUBOStorage}};

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = 0;
        pool_info.maxSets = maxSets;
        pool_info.poolSizeCount = (uint32_t)sizes.size();
        pool_info.pPoolSizes = sizes.data();

        VK_CHECK(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_pool));
    }
    void DescriptorManager::create_set_layout(uint32_t layoutSetIndex, VkDescriptorSetLayoutBinding *bindings, uint32_t bindingCount)
    {
        VkDescriptorSetLayout layout;
        VkDescriptorSetLayoutCreateInfo setinfo = {};
        setinfo.bindingCount = bindingCount;
        setinfo.flags = 0;
        setinfo.pNext = nullptr;
        setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setinfo.pBindings = bindings;

        VK_CHECK(vkCreateDescriptorSetLayout(m_device, &setinfo, nullptr, &layout));

        m_layouts[layoutSetIndex] = layout;
    }
    void DescriptorManager::allocate_descriptor_set(uint32_t layoutSetIndex, VkDescriptorSet *descriptor)
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_layouts[layoutSetIndex];

        VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo,
                                          descriptor));
    }
    void DescriptorManager::set_descriptor_write(VkBuffer buffer, VkDeviceSize dataSize, VkDeviceSize readOffset, VkDescriptorSet descriptor, VkDescriptorType type, uint32_t binding)
    {
        VkDescriptorBufferInfo info;
        info.buffer = buffer;
        info.offset = readOffset;
        info.range = dataSize;

        VkWriteDescriptorSet writeSetting = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptor, &info, binding);

        vkUpdateDescriptorSets(m_device, 1, &writeSetting, 0, nullptr);
    }
    void DescriptorManager::cleanup()
    {
        for (auto &layout : m_layouts)
        {
            vkDestroyDescriptorSetLayout(m_device, layout.second, nullptr);
        }
        vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    }
}