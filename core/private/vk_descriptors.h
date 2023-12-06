#ifndef VK_DESCRIPTORS
#define VK_DESCRIPTORS

#include <unordered_map>
#include "vk_core.h"
#include "vk_initializers.h"

namespace vke
{
    struct Descriptor{
        VkDescriptorSet descriptorSet;

        // Buffer* binded_buffers;
        uint32_t layoutIdx;


    };

    class DescriptorManager
    {
        VkDevice m_device;

    public:
        VkDescriptorPool m_pool{};
        std::unordered_map<uint32_t, VkDescriptorSetLayout> m_layouts;

        inline void init(VkDevice dvc) { m_device = dvc; }

        void create_pool(
            uint32_t numUBO,
            uint32_t numUBODynamic,
            uint32_t numUBOStorage,
            uint32_t maxSets);

        void create_set_layout(uint32_t layoutSetIndex, VkDescriptorSetLayoutBinding *bindings, uint32_t bindingCount);

        void allocate_descriptor_set(uint32_t layoutSetIndex, VkDescriptorSet *descriptor);

        void set_descriptor_write(VkBuffer buffer, VkDeviceSize dataSize,VkDeviceSize readOffset,VkDescriptorSet descriptor, VkDescriptorType type, uint32_t binding);

        // void bind_descriptor_set(descriptor , offset, etc);
        // void bindDescrpitprot to material
        void cleanup();
    };

    // Descriptor set pointing to buffers binded;????????
}

#endif