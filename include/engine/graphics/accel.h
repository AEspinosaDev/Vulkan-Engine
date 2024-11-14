/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef ACCEL_H
#define ACCEL_H

#include <engine/common.h>
#include <engine/graphics/extensions.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/vao.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {
/*
Bottom Level Acceleration structure
*/
struct BLAS {
    VkAccelerationStructureKHR handle       = VK_NULL_HANDLE;
    VkDevice                   device       = VK_NULL_HANDLE;
    VkDeviceAddress            deviceAdress = 0;

    Buffer buffer = {};

    void cleanup();
};
/*
Instance of a Bottom Level Acceleration Structure. Has a unique transform.
*/
struct BLASInstance {
    BLAS accel     = {};
    Mat4 transform = {};
};
/*
Top Level Acceleration structure
*/
struct TLAS {
    VkAccelerationStructureKHR handle       = VK_NULL_HANDLE;
    VkDevice                   device       = VK_NULL_HANDLE;
    VkDeviceAddress            deviceAdress = 0;

    Buffer buffer = {};
    bool binded = false;

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif