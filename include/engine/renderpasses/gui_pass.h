/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef GUI_PASS_H
#define GUI_PASS_H
#include <engine/core/renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

class GUIPass : public RenderPass
{
    ColorFormatType m_colorFormat;
    Mesh *m_vignette;

    GUIOverlay *m_gui;

    DescriptorManager m_descriptorManager{};
    DescriptorSet m_descriptorSet;

    Image m_outputBuffer;

public:
    GUIPass(VkExtent2D extent,
            uint32_t framebufferCount,
            ColorFormatType colorFormat, Mesh *vignette) : RenderPass(extent, framebufferCount, 1, true),
                                                           m_colorFormat(colorFormat), m_vignette(vignette) {}

    void init(VkDevice &device);

    void create_descriptors(VkDevice &device, VkPhysicalDevice &gpu, VmaAllocator &memory, uint32_t framesPerFlight);

    void create_pipelines(VkDevice &device, DescriptorManager &descriptorManager);

    void render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex = 0);

    void set_output_buffer(Image output);

    void cleanup(VkDevice &device, VmaAllocator &memory);

    inline void set_gui(GUIOverlay *gui) { m_gui = gui; }
};
VULKAN_ENGINE_NAMESPACE_END

#endif