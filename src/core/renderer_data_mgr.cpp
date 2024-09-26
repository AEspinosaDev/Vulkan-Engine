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
#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::upload_object_data(Scene *const scene)
{

	if (scene->get_active_camera() && scene->get_active_camera()->is_active())
	{
		std::vector<Mesh *> meshes;
		std::vector<Mesh *> blendMeshes;

		for (Mesh *m : scene->get_meshes())
		{
			if (m->get_material())
				m->get_material()->get_parameters().blending ? blendMeshes.push_back(m) : meshes.push_back(m);
		}

		// Calculate distance
		if (!blendMeshes.empty())
		{

			std::map<float, Mesh *> sorted;
			for (unsigned int i = 0; i < blendMeshes.size(); i++)
			{
				float distance = glm::distance(scene->get_active_camera()->get_position(), blendMeshes[i]->get_position());
				sorted[distance] = blendMeshes[i];
			}

			// SECOND = TRANSPARENT OBJECTS SORTED FROM NEAR TO FAR
			for (std::map<float, Mesh *>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
			{
				meshes.push_back(it->second);
			}
			scene->set_meshes(meshes);
		}

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

					// ObjectUniforms objectData;
					ObjectUniforms objectData;
					objectData.model = m->get_model_matrix();
					objectData.otherParams1 = {m->is_affected_by_fog(), m->get_recive_shadows(), m->get_cast_shadows(), false};
					objectData.otherParams2 = {m->is_selected(), 0.0, 0.0, 0.0};
					m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory, &objectData, sizeof(ObjectUniforms), objectOffset);

					for (size_t i = 0; i < m->get_num_geometries(); i++)
					{
						// Object vertex buffer setup
						Geometry *g = m->get_geometry(i);
						if (!g->is_buffer_loaded())
						{
							Geometry::upload_buffers(m_device, m_memory, m_graphicsQueue, m_uploadContext, g);
							m_deletionQueue.push_function([=]()
														  { g->cleanup(m_memory); });
						}

						// Object material setup
						Material *mat = m->get_material(g->get_material_ID());
						if (mat)
							setup_material(mat);
						else
							setup_material(Material::DEBUG_MATERIAL);

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
		camera->set_projection(m_window->get_extent().width, m_window->get_extent().height);
	CameraUniforms camData;
	camData.view = camera->get_view();
	camData.proj = camera->get_projection();
	camData.viewProj = camera->get_projection() * camera->get_view();
	camData.position = Vec4(camera->get_position(), 0.0f);
	camData.screenExtent = {m_window->get_extent().width, m_window->get_extent().height};

	m_frames[m_currentFrame].globalUniformBuffer.upload_data(m_memory, &camData, sizeof(CameraUniforms), 0);
	static_cast<SSAOPass *>(m_renderPipeline.renderpasses[SSAO])->update_uniforms(m_memory, camData, {scene->get_ssao_radius(), scene->get_ssao_bias()}, utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu) + utils::pad_uniform_buffer_size(sizeof(Vec2), m_gpu));
	static_cast<CompositionPass *>(m_renderPipeline.renderpasses[COMPOSITION])->update_uniforms(m_memory);

	SceneUniforms sceneParams;
	sceneParams.fogParams = {camera->get_near(), camera->get_far(), scene->get_fog_intensity(), scene->is_fog_enabled()};
	sceneParams.fogColorAndSSAO = Vec4(scene->get_fog_color(), scene->is_ssao_enabled());
	sceneParams.SSAOtype = static_cast<int>(m_settings.occlusionType);
	sceneParams.emphasizeAO = false;
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
	sceneParams.numLights = static_cast<int>(lights.size());

	m_frames[m_currentFrame].globalUniformBuffer.upload_data(m_memory, &sceneParams, sizeof(SceneUniforms),
															 utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu));
}

void Renderer::setup_material(Material *const mat)
{
	if (!mat->m_textureDescriptor.allocated)
		m_descriptorMng.allocate_descriptor_set(DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, &mat->m_textureDescriptor);

	auto textures = mat->get_textures();
	for (auto pair : textures)
	{
		Texture *texture = pair.second;
		if (texture && texture->is_data_loaded())
		{
			// SET ACTUAL TEXTURE
			if (!texture->is_buffer_loaded())
			{
				Texture::upload_data(m_device, m_gpu, m_memory, m_graphicsQueue, m_uploadContext, texture);
				m_deletionQueue.push_function([=]()
											  { texture->m_image.cleanup(m_device, m_memory); });
			}

			// Set texture write
			if (!mat->get_texture_binding_state()[pair.first] || texture->is_dirty())
			{
				m_descriptorMng.set_descriptor_write(texture->m_image.sampler, texture->m_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->m_textureDescriptor, pair.first);
				mat->set_texture_binding_state(pair.first, true);
				texture->m_isDirty = false;
			}
		}
		else
		{
			// SET DUMMY TEXTURE
			if (!mat->get_texture_binding_state()[pair.first])
				m_descriptorMng.set_descriptor_write(Texture::DEBUG_TEXTURE->m_image.sampler, Texture::DEBUG_TEXTURE->m_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->m_textureDescriptor, pair.first);
			mat->set_texture_binding_state(pair.first, true);
		}
	}

	mat->m_isDirty = false;
}

void Renderer::init_resources()
{

	// Setup vignette vertex buffers
	Geometry::upload_buffers(m_device, m_memory, m_graphicsQueue, m_uploadContext, m_vignette->get_geometry());

	// Setup dummy texture in case materials dont have textures
	Texture::DEBUG_TEXTURE = new Texture();
	Texture::DEBUG_TEXTURE->load_image(ENGINE_RESOURCES_PATH "textures/dummy.jpg", false);
	Texture::DEBUG_TEXTURE->set_use_mipmaps(false);
	Texture::upload_data(m_device, m_gpu, m_memory, m_graphicsQueue, m_uploadContext, Texture::DEBUG_TEXTURE);

	set_renderpass_resources();
}

void Renderer::set_renderpass_resources()
{
	size_t i = 0;
	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		pass->init_resources(m_device, m_gpu, m_memory, m_graphicsQueue, m_uploadContext);

		if (i == GEOMETRY)
			static_cast<SSAOPass *>(m_renderPipeline.renderpasses[SSAO])->set_g_buffer(pass->get_attachments()[0].image, pass->get_attachments()[1].image);
		if (i == SSAO)
			static_cast<SSAOBlurPass *>(m_renderPipeline.renderpasses[SSAO_BLUR])->set_ssao_buffer(pass->get_attachments()[0].image);

		i++;
	}
	static_cast<CompositionPass *>(m_renderPipeline.renderpasses[COMPOSITION])->set_g_buffer(m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[0].image, m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[1].image, m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[2].image, m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[3].image, m_descriptorMng);

	if (m_settings.AAtype == AntialiasingType::FXAA)
		static_cast<FXAAPass *>(m_renderPipeline.renderpasses[DefaultRenderPasses::FXAA])->set_output_buffer(m_settings.renderingType == RendererType::TDEFERRED ? m_renderPipeline.renderpasses[COMPOSITION]->get_attachments()[0].image : m_renderPipeline.renderpasses[FORWARD]->get_attachments()[0].image);

	// Set global textures descriptor writes
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[SHADOW]->get_attachments().front().image.sampler,
											 m_renderPipeline.renderpasses[SHADOW]->get_attachments().front().image.view,
											 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &m_frames[i].globalDescriptor, 2);

		if (m_settings.occlusionType == AmbientOcclusionType::SSAO)
			m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[SSAO_BLUR]->get_attachments().front().image.sampler,
												 m_renderPipeline.renderpasses[SSAO_BLUR]->get_attachments().front().image.view,
												 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_frames[i].globalDescriptor, 3);
		else
			m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[GEOMETRY]->get_attachments().front().image.sampler,
												 m_renderPipeline.renderpasses[GEOMETRY]->get_attachments().front().image.view,
												 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_frames[i].globalDescriptor, 3);
	}
}
VULKAN_ENGINE_NAMESPACE_END