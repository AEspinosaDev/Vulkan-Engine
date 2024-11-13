/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <engine/common.h>
#include <engine/graphics/utilities/initializers.h>
#include <engine/graphics/vk_renderpass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

class Framebuffer
{
    VkFramebuffer m_handle = VK_NULL_HANDLE;
    VkDevice      m_device = VK_NULL_HANDLE;
    uint32_t      m_layers = 1;

  public:
    Framebuffer() {
    }

    inline VkFramebuffer& get_handle()  {
        return m_handle;
    }

    inline uint32_t get_layer_count() const {
        return m_layers;
    }

    void init(VulkanRenderPass& renderpass, std::vector<Attachment>& attachments, uint32_t layers = 1);

    void cleanup();
};

} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END
#endif