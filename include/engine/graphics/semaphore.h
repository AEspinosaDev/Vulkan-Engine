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
    ~Semaphore() {
        cleanup();
    }

    inline VkSemaphore get_handle() const {
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
    ~Fence() {
        cleanup();
    }

    inline VkFence get_handle() const {
        return m_handle;
    }

    void init(VkDevice device);
    void cleanup();
    void reset();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif