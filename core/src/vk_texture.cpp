#define STB_IMAGE_IMPLEMENTATION
#include "engine/vk_texture.h"

namespace vke
{
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
        loaded = true;
        return loaded;
    }
    void Texture::create_sampler(VkDevice device)
    {
        VkSamplerCreateInfo samplerInfo = vkinit::sampler_create_info(VK_FILTER_NEAREST);
        vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler);
    }

}