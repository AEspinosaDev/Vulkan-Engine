#ifndef VULKAN_EXT_H
#define VULKAN_EXT_H
/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
/*

/////////////////////////////////////////////////////////////

DECLARATION OF FUNCTION POINTERS RELATED TO VULKAN EXTENSIONS

/////////////////////////////////////////////////////////////

*/

#include <engine/common.h>

// Global function pointers
extern PFN_vkCmdSetRasterizationSamplesEXT            vkCmdSetRasterizationSamples;
extern PFN_vkCmdSetPolygonModeEXT                     vkCmdSetPolygonMode;
extern PFN_vkCreateAccelerationStructureKHR           vkCreateAccelerationStructure;
extern PFN_vkDestroyAccelerationStructureKHR          vkDestroyAccelerationStructure;
extern PFN_vkGetAccelerationStructureBuildSizesKHR    vkGetAccelerationStructureBuildSizes;
extern PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddress;
extern PFN_vkCmdBuildAccelerationStructuresKHR        vkCmdBuildAccelerationStructures;
extern PFN_vkBuildAccelerationStructuresKHR           vkBuildAccelerationStructures;
extern PFN_vkSetDebugUtilsObjectNameEXT               vkSetDebugUtilsObjectName;

void load_extensions(VkDevice& device, VkInstance& instance);

#endif