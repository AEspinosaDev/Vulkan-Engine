#include <engine/graphics/extensions.h>

PFN_vkCmdSetRasterizationSamplesEXT            vkCmdSetRasterizationSamples            = nullptr;
PFN_vkCmdSetPolygonModeEXT                     vkCmdSetPolygonMode                     = nullptr;
PFN_vkCreateAccelerationStructureKHR           vkCreateAccelerationStructure           = nullptr;
PFN_vkDestroyAccelerationStructureKHR          vkDestroyAccelerationStructure          = nullptr;
PFN_vkGetAccelerationStructureBuildSizesKHR    vkGetAccelerationStructureBuildSizes    = nullptr;
PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddress = nullptr;
PFN_vkCmdBuildAccelerationStructuresKHR        vkCmdBuildAccelerationStructures        = nullptr;
PFN_vkBuildAccelerationStructuresKHR           vkBuildAccelerationStructures           = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT               vkSetDebugUtilsObjectName               = nullptr;

void load_extensions(VkDevice& device, VkInstance& instance) {

    vkCmdSetPolygonMode = (PFN_vkCmdSetPolygonModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT");

    if (!vkCmdSetPolygonMode)
    {
        LOG_ERROR("Failed to load vkCmdSetPolygonModeEXT!");
    }

    vkCmdSetRasterizationSamples =
        (PFN_vkCmdSetRasterizationSamplesEXT)vkGetDeviceProcAddr(device, "vkCmdSetRasterizationSamplesEXT");

    if (!vkCmdSetRasterizationSamples)
    {
        LOG_ERROR("Failed to load vkCmdSetRasterizationSamplesEXT!");
    }

    vkCreateAccelerationStructure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
        vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));

    if (!vkCreateAccelerationStructure)
    {
        LOG_ERROR("Failed to load vkCreateAccelerationStructureKHR!");
    }
    vkDestroyAccelerationStructure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
        vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));

    if (!vkDestroyAccelerationStructure)
    {
        LOG_ERROR("Failed to load vkDestroyAccelerationStructureKHR!");
    }
    vkGetAccelerationStructureBuildSizes = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));

    if (!vkGetAccelerationStructureBuildSizes)
    {
        LOG_ERROR("Failed to load vkGetAccelerationStructureBuildSizesKHR!");
    }
    vkGetAccelerationStructureDeviceAddress = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
        vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));

    if (!vkGetAccelerationStructureDeviceAddress)
    {
        LOG_ERROR("Failed to load vkGetAccelerationStructureDeviceAddressKHR!");
    }
    vkCmdBuildAccelerationStructures = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
        vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));

    if (!vkCmdBuildAccelerationStructures)
    {
        LOG_ERROR("Failed to load vkCmdBuildAccelerationStructuresKHR!");
    }
    vkBuildAccelerationStructures = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(
        vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));

    if (!vkBuildAccelerationStructures)
    {
        LOG_ERROR("Failed to load vkBuildAccelerationStructuresKHR!");
    }

    vkSetDebugUtilsObjectName =
        reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
    if (!vkSetDebugUtilsObjectName)
    {
        LOG_ERROR("Failed to load vkSetDebugUtilsObjectNameEXT!");
    }
}