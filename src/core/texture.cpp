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
							   {m_tmpCache = stbi_load(fileName.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);if(m_tmpCache)m_loaded=true; m_isDirty = true; });
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

void Texture::upload_data(VkDevice &device, VkPhysicalDevice &gpu, VmaAllocator &memory, VkQueue &gfxQueue, utils::UploadContext &uploadContext, Texture *const t)
{
	// INIT IMAGE AND CREATE VIEW
	VkExtent3D extent = {(uint32_t)t->m_width,
						 (uint32_t)t->m_height,
						 (uint32_t)t->m_depth};

	t->m_image.init(memory, (VkFormat)t->m_settings.format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent, t->m_settings.useMipmaps, VK_SAMPLE_COUNT_1_BIT);
	t->m_image.create_view(device, VK_IMAGE_ASPECT_COLOR_BIT);

	Buffer stagingBuffer;

	void *pixel_ptr = t->m_tmpCache;
	VkDeviceSize imageSize = t->m_width * t->m_height * t->m_depth * Image::BYTES_PER_PIXEL;

	stagingBuffer.init(memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	stagingBuffer.upload_data(memory, pixel_ptr, static_cast<size_t>(imageSize));

	// free(t->m_tmpCache);

	uploadContext.immediate_submit(device, gfxQueue, [&](VkCommandBuffer cmd)
								   { t->m_image.upload_image(cmd, &stagingBuffer); });

	stagingBuffer.cleanup(memory);

	t->m_buffer_loaded = true;

	// GENERATE MIPMAPS
	if (t->m_settings.useMipmaps)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(gpu, t->m_image.format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		uploadContext.immediate_submit(device, gfxQueue, [&](VkCommandBuffer cmd)
									   { t->m_image.generate_mipmaps(cmd); });
	}
	// CREATE SAMPLER
	t->m_image.create_sampler(device, (VkFilter)t->m_settings.filter,
							  VK_SAMPLER_MIPMAP_MODE_LINEAR,
							  (VkSamplerAddressMode)t->m_settings.adressMode,
							  (float)t->m_settings.minMipLevel,
							  t->m_settings.useMipmaps ? (float)t->m_image.mipLevels : 1.0f,
							  t->m_settings.anisotropicFilter, utils::get_gpu_properties(gpu).limits.maxSamplerAnisotropy);
}

VULKAN_ENGINE_NAMESPACE_END