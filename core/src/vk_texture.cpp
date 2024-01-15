#define STB_IMAGE_IMPLEMENTATION
#include "engine/vk_texture.h"

namespace vke
{
    Texture *Texture::DEBUG_TEXTURE = nullptr;

    bool Texture::load_image(std::string fileName)
    {
        if (m_tmpCache)
            return false;

        m_tmpCache = stbi_load(fileName.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);

        if (!m_tmpCache)
        {
            DEBUG_LOG("Failed to load texture file" + fileName);
            return false;
        };

        DEBUG_LOG("Texture loaded successfully");
        m_loaded = true;
        return m_loaded;
    }
   

    void Texture::cleanup(VkDevice &device, VmaAllocator &memory)
    {
        m_image.cleanup(device, memory);
        vkDestroySampler(device, m_sampler, VK_NULL_HANDLE);
    }

}