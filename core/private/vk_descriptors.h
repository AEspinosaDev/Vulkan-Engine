#ifndef VK_DESCRIPTORS
#define VK_DESCRIPTORS

#include <unordered_map>
#include "vk_core.h"

namespace vke
{

    class DescriptorAllocator
    {

    public:
        struct PoolSizes
        {
            std::vector<std::pair<VkDescriptorType, float>> sizes =
                {
                    {VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f},
                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f},
                    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f},
                    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f},
                    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f},
                    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f},
                    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f}};
        };

        void reset_pools();

        bool allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout);

        void init(VkDevice newDevice);

        void cleanup();

        VkDevice device;

    private:
        VkDescriptorPool grab_pool();

        VkDescriptorPool m_currentPool{VK_NULL_HANDLE};
        PoolSizes m_descriptorSizes;
        std::vector<VkDescriptorPool> m_usedPools;
        std::vector<VkDescriptorPool> m_freePools;
    };

    class DescriptorLayoutCache
    {

    public:
        void init(VkDevice newDevice);
        void cleanup();

        VkDescriptorSetLayout create_descriptor_layout(VkDescriptorSetLayoutCreateInfo *info);

        struct DescriptorLayoutInfo
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            bool operator==(const DescriptorLayoutInfo &other) const;

            size_t hash() const;
        };

    private:
        struct DescriptorLayoutHash
        {

            std::size_t operator()(const DescriptorLayoutInfo &k) const
            {
                return k.hash();
            }
        };

        std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> m_layoutCache;
        VkDevice m_device;
    };
    class DescriptorBuilder
    {
    private:
        std::vector<VkWriteDescriptorSet> m_writes;
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;

        DescriptorLayoutCache *m_cache;
        DescriptorAllocator *m_alloc;

    public:
        static DescriptorBuilder begin(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator);

        DescriptorBuilder &bind_buffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
        DescriptorBuilder &bind_image(uint32_t binding, VkDescriptorImageInfo *imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

        bool build(VkDescriptorSet &set, VkDescriptorSetLayout &layout);
        bool build(VkDescriptorSet &set);
    };
}

#endif