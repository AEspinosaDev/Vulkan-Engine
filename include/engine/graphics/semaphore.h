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

/*
To be populated by device class with create_semaphore()
*/
struct Semaphore
{
    VkSemaphore handle = VK_NULL_HANDLE;
    VkDevice    device = VK_NULL_HANDLE;


    void cleanup();
   
};

/*
To be populated by device class with create_fence()
*/
struct Fence
{
    VkFence  handle = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

   
    void reset();
    void wait(uint64_t timeout = UINT64_MAX);
    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif