#define STB_IMAGE_IMPLEMENTATION
#include <engine/vk_texture.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

Texture *Texture::DEBUG_TEXTURE = nullptr;

void Texture::load_image(std::string fileName, bool asyncCall)
{
	if (m_tmpCache)
		return;

	if (asyncCall)
	{
		std::thread loadThread([=]()
							   {m_tmpCache = stbi_load(fileName.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);if(m_tmpCache)m_loaded=true; m_isDirty = true;});
		loadThread.detach();
	}
	else
	{
		m_tmpCache = stbi_load(fileName.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);

		if (!m_tmpCache)
		{
#ifndef NDEBUG
			DEBUG_LOG("Failed to load texture file" + fileName);
#endif
			return;
		};
#ifndef NDEBUG
		DEBUG_LOG("Texture loaded successfully");
#endif // DEBUG

		m_loaded = true;
	}
}

void Texture::cleanup(VkDevice &device, VmaAllocator &memory)
{
	m_image.cleanup(device, memory);
	vkDestroySampler(device, m_sampler, VK_NULL_HANDLE);
}

VULKAN_ENGINE_NAMESPACE_END