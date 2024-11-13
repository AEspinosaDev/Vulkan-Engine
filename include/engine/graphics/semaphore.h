/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
////////////////////////////////////////////
// SEMAPHORE AND FENCES
///////////////////////////////////////////
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <engine/graphics/utilities/initializers.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

class Semaphore
{
    VkSemaphore m_handle = VK_NULL_HANDLE;
    VkDevice    m_device = VK_NULL_HANDLE;

  public:
    Semaphore() {
    }

    inline VkSemaphore& get_handle() {
        return m_handle;
    }

    void init(VkDevice device);
    void cleanup();
};

class Fence
{
    VkFence  m_handle = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

  public:
    Fence() {
    }

    inline VkFence& get_handle() {
        return m_handle;
    }

    void init(VkDevice device);
    void cleanup();
    void reset();
    void wait(uint64_t timeout = UINT64_MAX);
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif