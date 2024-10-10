#include <engine/graphics/extensions.h>

PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamples = nullptr;
PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonMode = nullptr;

void load_EXT_functions(VkDevice &device)
{

    vkCmdSetPolygonMode = (PFN_vkCmdSetPolygonModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT");

    if (!vkCmdSetPolygonMode)
    {
        ERR_LOG("Failed to load vkCmdSetPolygonModeEXT!");
    }

    vkCmdSetRasterizationSamples =
        (PFN_vkCmdSetRasterizationSamplesEXT)vkGetDeviceProcAddr(device, "vkCmdSetRasterizationSamplesEXT");

    if (!vkCmdSetRasterizationSamples)
    {
        ERR_LOG("Failed to load vkCmdSetRasterizationSamplesEXT!");
    }
}