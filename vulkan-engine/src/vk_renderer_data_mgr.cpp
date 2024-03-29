/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

	////////////////////////////////////////////////////////////////////////////////////

	In this Renderer's module you will find:

	Implementation of functions focused on managing the uploading to the GPU and caching of data needed for 
	rendering.

	////////////////////////////////////////////////////////////////////////////////////
*/
#include <engine/vk_renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::upload_geometry_data(Geometry *const g)
{
	// Should be executed only once if geometry data is not changed

	// Staging vertex buffer (CPU only)
	size_t vboSize = sizeof(g->m_vertexData[0]) * g->m_vertexData.size();
	Buffer vboStagingBuffer;
	vboStagingBuffer.init(m_memory, vboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	vboStagingBuffer.upload_data(m_memory, g->m_vertexData.data(), vboSize);

	// GPU vertex buffer
	g->m_vbo->init(m_memory, vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	immediate_submit([=](VkCommandBuffer cmd)
					 {
				VkBufferCopy copy;
				copy.dstOffset = 0;
				copy.srcOffset = 0;
				copy.size = vboSize;
				vkCmdCopyBuffer(cmd, vboStagingBuffer.buffer, g->m_vbo->buffer, 1, &copy); });

	m_deletionQueue.push_function([=]()
								  { g->m_vbo->cleanup(m_memory); });
	vboStagingBuffer.cleanup(m_memory);

	if (g->indexed)
	{
		// Staging index buffer (CPU only)
		size_t iboSize = sizeof(g->m_vertexIndex[0]) * g->m_vertexIndex.size();
		Buffer iboStagingBuffer;
		iboStagingBuffer.init(m_memory, iboSize, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		iboStagingBuffer.upload_data(m_memory, g->m_vertexIndex.data(), iboSize);

		// GPU index buffer
		g->m_ibo->init(m_memory, iboSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		immediate_submit([=](VkCommandBuffer cmd)
						 {

					VkBufferCopy index_copy;
					index_copy.dstOffset = 0;
					index_copy.srcOffset = 0;
					index_copy.size = iboSize;
					vkCmdCopyBuffer(cmd, iboStagingBuffer.buffer, g->m_ibo->buffer, 1, &index_copy); });
		m_deletionQueue.push_function([=]()
									  { g->m_ibo->cleanup(m_memory); });
		iboStagingBuffer.cleanup(m_memory);
	}

	g->buffer_loaded = true;
}
void Renderer::upload_object_data(Scene *const scene)
{

	if (scene->get_active_camera() && scene->get_active_camera()->is_active())
	{

		unsigned int mesh_idx = 0;
		for (Mesh *m : scene->get_meshes())
		{
			if (m) // If mesh exists
			{
				if (m->is_active() &&																	  // Check if is active
					m->get_num_geometries() > 0 &&														  // Check if has geometry
					m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum())) // Check if is inside frustrum
				{
					// Offset calculation
					uint32_t objectOffset = m_frames[m_currentFrame].objectUniformBuffer.strideSize * mesh_idx;
					uint32_t globalOffset = m_globalUniformsBuffer.strideSize * m_currentFrame;

					// ObjectUniforms objectData;
					ObjectUniforms objectData;
					objectData.model = m->get_model_matrix();
					objectData.otherParams = {m->is_affected_by_fog(), m->get_recive_shadows(), m->get_cast_shadows(), false};
					m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory, &objectData, sizeof(ObjectUniforms), objectOffset);

					for (size_t i = 0; i < m->get_num_geometries(); i++)
					{
						// Object vertex buffer setup
						Geometry *g = m->get_geometry(i);
						if (!g->buffer_loaded)
							upload_geometry_data(g);

						// Object material setup
						Material *mat = m->get_material(g->m_materialID);
						if (!mat)
							setup_material(Material::DEBUG_MATERIAL);
						setup_material(mat);

						// ObjectUniforms materialData;
						MaterialUniforms materialData = mat->get_uniforms();
						m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory, &materialData, sizeof(MaterialUniforms), objectOffset + utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_gpu));
					}
				}
			}
			mesh_idx++;
		}
	}
}

void Renderer::upload_global_data(Scene *const scene)
{
	Camera *camera = scene->get_active_camera();
	if (camera->is_dirty())
		camera->set_projection(m_window->get_extent()->width, m_window->get_extent()->height);
	CameraUniforms camData;
	camData.view = camera->get_view();
	camData.proj = camera->get_projection();
	camData.viewProj = camera->get_projection() * camera->get_view();

	m_globalUniformsBuffer.upload_data(m_memory, &camData, sizeof(CameraUniforms),
									   m_globalUniformsBuffer.strideSize * m_currentFrame);

	SceneUniforms sceneParams;
	sceneParams.fogParams = {camera->get_near(), camera->get_far(), scene->get_fog_intensity(), scene->is_fog_active()};
	sceneParams.fogColor = Vec4(scene->get_fog_color(), 1.0f);
	sceneParams.ambientColor = Vec4(scene->get_ambient_color(), scene->get_ambient_intensity());

	std::vector<Light *> lights = scene->get_lights();
	if (lights.size() > VK_MAX_LIGHTS)
		std::sort(lights.begin(), lights.end(), [=](Light *a, Light *b)
				  { return math::length(a->get_position() - camera->get_position()) < math::length(b->get_position() - camera->get_position()); });

	size_t lightIdx{0};
	for (Light *l : lights)
	{
		if (l->is_active())
		{
			sceneParams.lightUniforms[lightIdx] = l->get_uniforms(camera->get_view());
			Mat4 depthProjectionMatrix = math::perspective(math::radians(l->m_shadow.fov), 1.0f, l->m_shadow.nearPlane, l->m_shadow.farPlane);
			Mat4 depthViewMatrix = math::lookAt(l->m_transform.position, l->m_shadow.target, Vec3(0, 1, 0));
			sceneParams.lightUniforms[lightIdx].viewProj = depthProjectionMatrix * depthViewMatrix;
			lightIdx++;
		}
		if (lightIdx >= VK_MAX_LIGHTS)
			break;
	}
	sceneParams.numLights = (int)lights.size();

	m_globalUniformsBuffer.upload_data(m_memory, &sceneParams, sizeof(SceneUniforms),
									   m_globalUniformsBuffer.strideSize * m_currentFrame + utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu));
}

void Renderer::setup_material(Material *const mat)
{
	if (!mat->m_shaderPass)
	{
		mat->m_shaderPass = m_shaderPasses[mat->m_shaderPassID];
	}
	if (!mat->m_textureDescriptor.allocated)
	{
		m_descriptorMng.allocate_descriptor_set(DescriptorLayoutType::TEXTURE_LAYOUT, &mat->m_textureDescriptor);

		// Set Shadow Map write
		m_descriptorMng.set_descriptor_write(m_renderPasses[SHADOW].textureAttachments.front()->m_sampler, m_renderPasses[SHADOW].textureAttachments.front()->m_image.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &mat->m_textureDescriptor, 0);
	}

	auto textures = mat->get_textures();
	for (auto pair : textures)
	{
		Texture *texture = pair.second;
		if (texture && texture->is_data_loaded())
		{
			// SET ACTUAL TEXTURE
			if (!texture->is_buffer_loaded())
				upload_texture(texture);

			// Set texture write
			if (!mat->get_texture_binding_state()[pair.first] || texture->is_dirty())
			{
				m_descriptorMng.set_descriptor_write(texture->m_sampler, texture->m_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->m_textureDescriptor, pair.first + 1);
				mat->set_texture_binding_state(pair.first, true);
				texture->m_isDirty = false;
			}
		}
		else
		{
			// SET DUMMY TEXTURE
			if (!mat->get_texture_binding_state()[pair.first])
				m_descriptorMng.set_descriptor_write(Texture::DEBUG_TEXTURE->m_sampler, Texture::DEBUG_TEXTURE->m_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->m_textureDescriptor, pair.first + 1);
			mat->set_texture_binding_state(pair.first, true);
		}
	}

	mat->m_isDirty = false;
}

void Renderer::upload_texture(Texture *const t)
{
	VkExtent3D extent = {(uint32_t)t->m_width,
						 (uint32_t)t->m_height,
						 (uint32_t)t->m_depth};

	t->m_image.init(m_memory, (VkFormat)t->m_settings.format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent, t->m_settings.useMipmaps, VK_SAMPLE_COUNT_1_BIT);
	t->m_image.create_view(m_device, VK_IMAGE_ASPECT_COLOR_BIT);

	Buffer stagingBuffer;

	void *pixel_ptr = t->m_tmpCache;
	VkDeviceSize imageSize = t->m_width * t->m_height * t->m_depth * Image::BYTES_PER_PIXEL;

	stagingBuffer.init(m_memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	stagingBuffer.upload_data(m_memory, pixel_ptr, static_cast<size_t>(imageSize));

	free(t->m_tmpCache);

	immediate_submit([&](VkCommandBuffer cmd)
					 { t->m_image.upload_image(cmd, &stagingBuffer); });

	stagingBuffer.cleanup(m_memory);

	t->m_buffer_loaded = true;

	// GENERATE MIPMAPS
	if (t->m_settings.useMipmaps)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_gpu, t->m_image.format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		immediate_submit([&](VkCommandBuffer cmd)
						 { t->m_image.generate_mipmaps(cmd); });
	}
	// CREATE SAMPLER
	VkSamplerCreateInfo samplerInfo = init::sampler_create_info((VkFilter)t->m_settings.filter,
																VK_SAMPLER_MIPMAP_MODE_LINEAR,
																(float)t->m_settings.minMipLevel,
																t->m_settings.useMipmaps ? (float)t->m_image.mipLevels : 1.0f,
																t->m_settings.anisotropicFilter, utils::get_gpu_properties(m_gpu).limits.maxSamplerAnisotropy,
																(VkSamplerAddressMode)t->m_settings.adressMode);
	vkCreateSampler(m_device, &samplerInfo, nullptr, &t->m_sampler);

	m_deletionQueue.push_function([=]()
								  { t->cleanup(m_device, m_memory); });
}


void Renderer::init_resources()
{

	// // m_shadowsTexture = new Texture();
	// // m_shadowsTexture->m_image = m_renderPasses[SHADOW].textureAttachments[0];
	
	// VkSamplerCreateInfo sampler = init::sampler_create_info(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, 1.0f, false, 1.0f, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	// sampler.maxAnisotropy = 1.0f;
	// sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	// VK_CHECK(vkCreateSampler(m_device, &sampler, nullptr, &m_shadowsTexture->m_sampler));

	// m_deletionQueue.push_function([=]()
	// 							  { m_shadowsTexture->cleanup(m_device, m_memory); });

	Texture::DEBUG_TEXTURE = new Texture();
	std::string engineMeshDir(VK_TEXTURE_DIR);
	Texture::DEBUG_TEXTURE->load_image(engineMeshDir + "dummy_.jpg", false);
	Texture::DEBUG_TEXTURE->set_use_mipmaps(false);
	upload_texture(Texture::DEBUG_TEXTURE);

	// Material::DEBUG_MATERIAL = new B
}

VULKAN_ENGINE_NAMESPACE_END