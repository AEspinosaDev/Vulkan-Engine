#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

Texture *Texture::DEBUG_TEXTURE = nullptr;


void Texture::load_image(std::string fileName, bool asyncCall)
{
	if (m_tmpCache)
		return;

	if (asyncCall)
	{
		std::thread loadThread([=]()
							   {
								   int w, h, ch;
								   m_tmpCache = stbi_load(fileName.c_str(), &w, &h, &ch, STBI_rgb_alpha);
								   if (m_tmpCache)
									   m_loaded = true;
								   m_isDirty = true;
								   m_channels = ch;
								   m_image.extent = {(unsigned int)w,(unsigned int) h, 1}; });
		loadThread.detach();
	}
	else
	{
		int w, h, ch;
		m_tmpCache = stbi_load(fileName.c_str(), &w, &h, &ch, STBI_rgb_alpha);
		m_image.extent = {(unsigned int)w, (unsigned int)h, 1};
		m_channels = ch;

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

VULKAN_ENGINE_NAMESPACE_END