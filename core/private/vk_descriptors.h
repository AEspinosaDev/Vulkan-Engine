#ifndef VK_DESCRIPTORS
#define VK_DESCRIPTORS

#include <unordered_map>
#include "vk_core.h"
#include "vk_initializers.h"
#include "vk_buffer.h"

namespace vke
{
	struct DescriptorSet
	{
		VkDescriptorSet descriptorSet{};

		std::vector<Buffer*> binded_buffers;
		uint32_t layoutID;
		uint32_t bindings;
		bool isDynamic;

		void bind(VkCommandBuffer commandBuffer,
			VkPipelineBindPoint pipelineBindPoint,
			VkPipelineLayout pipelineLayout,
			uint32_t firstSet,
			const std::vector<uint32_t> offsets);
	};

	class DescriptorManager
	{
		VkDevice m_device;
		VkDescriptorPool m_pool{};
		std::unordered_map<uint32_t, VkDescriptorSetLayout> m_layouts;

	public:
		inline void init(VkDevice &dvc) { m_device = dvc; }

		void create_pool(
			uint32_t numUBO,
			uint32_t numUBODynamic,
			uint32_t numUBOStorage,
			uint32_t numImageCombined,
			uint32_t maxSets);

		void set_layout(uint32_t layoutSetIndex, VkDescriptorSetLayoutBinding* bindings, uint32_t bindingCount, VkDescriptorSetLayoutCreateFlags flags = 0);

		inline VkDescriptorSetLayout get_layout(uint32_t layoutSetIndex) { return m_layouts[layoutSetIndex]; }

		void allocate_descriptor_set(uint32_t layoutSetIndex, DescriptorSet* descriptor);

		void set_descriptor_write(Buffer* buffer, VkDeviceSize dataSize, VkDeviceSize readOffset, DescriptorSet* descriptor, VkDescriptorType type, uint32_t binding);

		void set_descriptor_write(VkSampler sampler, VkImageView imageView, VkImageLayout layout, DescriptorSet* descriptor);

		void bind_descriptor_sets(VkCommandBuffer commandBuffer,
			VkPipelineBindPoint pipelineBindPoint,
			VkPipelineLayout pipelineLayout,
			uint32_t firstSet,
			const std::vector<DescriptorSet> descriptorSets);

		void cleanup();
	};

}

#endif