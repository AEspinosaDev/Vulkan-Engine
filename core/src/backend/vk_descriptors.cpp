#include "vk_descriptors.h"

namespace vke
{
    void DescriptorAllocator::init(VkDevice newDevice)
    {
        device = newDevice;
    }

    void DescriptorAllocator::cleanup()
    {
        // delete every pool held
        for (auto p : m_freePools)
        {
            vkDestroyDescriptorPool(device, p, nullptr);
        }
        for (auto p : m_usedPools)
        {
            vkDestroyDescriptorPool(device, p, nullptr);
        }
    }

    VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes &poolSizes, int count, VkDescriptorPoolCreateFlags flags)
    {
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(poolSizes.sizes.size());
        for (auto sz : poolSizes.sizes)
        {
            sizes.push_back({sz.first, uint32_t(sz.second * count)});
        }
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = flags;
        pool_info.maxSets = count;
        pool_info.poolSizeCount = (uint32_t)sizes.size();
        pool_info.pPoolSizes = sizes.data();

        VkDescriptorPool descriptorPool;
        VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool));

        return descriptorPool;
    }

    VkDescriptorPool DescriptorAllocator::grab_pool()
    {
        // there are reusable pools available
        if (m_freePools.size() > 0)
        {
            VkDescriptorPool pool = m_freePools.back();
            m_freePools.pop_back();
            return pool;
        }
        else
        {
            // no pools available, so create a new one
            return createPool(device, m_descriptorSizes, 1000, 0);
        }
    }

    bool DescriptorAllocator::allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout)
    {
        // initialize the currentPool handle if it's null
        if (m_currentPool == VK_NULL_HANDLE)
        {
            m_currentPool = grab_pool();
            m_usedPools.push_back(m_currentPool);
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.pSetLayouts = &layout;
        allocInfo.descriptorPool = m_currentPool;
        allocInfo.descriptorSetCount = 1;

        // try to allocate the descriptor set
        VkResult allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
        bool needReallocate = false;

        switch (allocResult)
        {
        case VK_SUCCESS:
            // all good, return
            return true;
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            // reallocate pool
            needReallocate = true;
            break;
        default:
            // unrecoverable error
            return false;
        }

        if (needReallocate)
        {
            // allocate a new pool and retry
            m_currentPool = grab_pool();
            m_usedPools.push_back(m_currentPool);

            allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);

            // if it still fails then we have big issues
            if (allocResult == VK_SUCCESS)
            {
                return true;
            }
        }

        return false;
    }

    void DescriptorAllocator::reset_pools()
    {
        for (auto p : m_usedPools)
        {
            VK_CHECK(vkResetDescriptorPool(device, p, 0));
            m_freePools.push_back(p);
        }
        m_usedPools.clear();
        m_currentPool = VK_NULL_HANDLE;
    }

    void DescriptorLayoutCache::init(VkDevice newDevice)
    {
        m_device = newDevice;
    }
    void DescriptorLayoutCache::cleanup()
    {
        // delete every descriptor layout held
        for (auto pair : m_layoutCache)
        {
            vkDestroyDescriptorSetLayout(m_device, pair.second, nullptr);
        }
    }

    VkDescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(VkDescriptorSetLayoutCreateInfo *info)
    {
        DescriptorLayoutInfo layoutinfo;
        layoutinfo.bindings.reserve(info->bindingCount);
        bool isSorted = true;
        int lastBinding = -1;

        // copy from the direct info struct into our own one
        for (int i = 0; i < info->bindingCount; i++)
        {
            layoutinfo.bindings.push_back(info->pBindings[i]);

            // check that the bindings are in strict increasing order
            if (info->pBindings[i].binding > lastBinding)
            {
                lastBinding = info->pBindings[i].binding;
            }
            else
            {
                isSorted = false;
            }
        }
        // sort the bindings if they aren't in order
        if (!isSorted)
        {
            std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(), [](VkDescriptorSetLayoutBinding &a, VkDescriptorSetLayoutBinding &b)
                      { return a.binding < b.binding; });
        }

        // try to grab from cache
        auto it = m_layoutCache.find(layoutinfo);
        if (it != m_layoutCache.end())
        {
            return (*it).second;
        }
        else
        {
            // create a new one (not found)
            VkDescriptorSetLayout layout;
            VK_CHECK(vkCreateDescriptorSetLayout(m_device, info, nullptr, &layout));

            // add to cache
            m_layoutCache[layoutinfo] = layout;
            return layout;
        }
    }

    bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo &other) const
    {
        if (other.bindings.size() != bindings.size())
        {
            return false;
        }
        else
        {
            // compare each of the bindings is the same. Bindings are sorted so they will match
            for (int i = 0; i < bindings.size(); i++)
            {
                if (other.bindings[i].binding != bindings[i].binding)
                {
                    return false;
                }
                if (other.bindings[i].descriptorType != bindings[i].descriptorType)
                {
                    return false;
                }
                if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
                {
                    return false;
                }
                if (other.bindings[i].stageFlags != bindings[i].stageFlags)
                {
                    return false;
                }
            }
            return true;
        }
    }
    size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
    {
        using std::hash;
        using std::size_t;

        size_t result = hash<size_t>()(bindings.size());

        for (const VkDescriptorSetLayoutBinding &b : bindings)
        {
            // pack the binding data into a single int64. Not fully correct but it's ok
            size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

            // shuffle the packed binding data and xor it with the main hash
            result ^= hash<size_t>()(binding_hash);
        }

        return result;
    }

    DescriptorBuilder DescriptorBuilder::begin(DescriptorLayoutCache *layoutCache, DescriptorAllocator *allocator)
    {

        DescriptorBuilder builder;

        builder.m_cache = layoutCache;
        builder.m_alloc = allocator;
        return builder;
    }

    DescriptorBuilder &DescriptorBuilder::bind_buffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
    {
        // create the descriptor binding for the layout
        VkDescriptorSetLayoutBinding newBinding{};

        newBinding.descriptorCount = 1;
        newBinding.descriptorType = type;
        newBinding.pImmutableSamplers = nullptr;
        newBinding.stageFlags = stageFlags;
        newBinding.binding = binding;

        m_bindings.push_back(newBinding);

        // create the descriptor write
        VkWriteDescriptorSet newWrite{};
        newWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        newWrite.pNext = nullptr;

        newWrite.descriptorCount = 1;
        newWrite.descriptorType = type;
        newWrite.pBufferInfo = bufferInfo;
        newWrite.dstBinding = binding;

        m_writes.push_back(newWrite);
        return *this;
    }

    bool DescriptorBuilder::build(VkDescriptorSet &set, VkDescriptorSetLayout &layout)
    {
        // build layout first
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr;

        layoutInfo.pBindings = m_bindings.data();
        layoutInfo.bindingCount = m_bindings.size();

        layout = m_cache->create_descriptor_layout(&layoutInfo);

        // allocate descriptor
        bool success = m_alloc->allocate(&set, layout);
        if (!success)
        {
            return false;
        };

        // write descriptor
        for (VkWriteDescriptorSet &w : m_writes)
        {
            w.dstSet = set;
        }

        vkUpdateDescriptorSets(m_alloc->device, m_writes.size(), m_writes.data(), 0, nullptr);

        return true;
    }

}