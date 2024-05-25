/*
	This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

	MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

	////////////////////////////////////////////////////////////////////////////////////

	In this Renderer's module you will find:

	Implementation of functions focused on managing the subyacent Vulkan graphic API, in order
	to boot it up, setting up the objects such as: swapchains, memory allocators, pipelines and descriptor sets.

	////////////////////////////////////////////////////////////////////////////////////
*/
#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::init_renderpasses()
{

	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
	const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

	// Creating default renderpasses
	ForwardPass *forwardPass = new ForwardPass(m_window->get_extent(),
											   totalImagesInFlight,
											   m_settings.colorFormat,
											   m_settings.depthFormat,
											   m_settings.AAtype);

	ShadowPass *shadowPass = new ShadowPass({SHADOW_RES, SHADOW_RES}, totalImagesInFlight, VK_MAX_LIGHTS, m_settings.depthFormat);

	GeometryPass *geometryPass = new GeometryPass(m_window->get_extent(), totalImagesInFlight, m_settings.depthFormat);

	SSAOPass *ssaoPass = new SSAOPass(m_window->get_extent(), totalImagesInFlight, m_vignette);

	SSAOBlurPass *ssaoBlurPass = new SSAOBlurPass(m_window->get_extent(), totalImagesInFlight, m_vignette);

	CompositionPass *compPass = new CompositionPass(m_window->get_extent(), totalImagesInFlight, m_settings.colorFormat, m_vignette, m_settings.AAtype == AntialiasingType::FXAA);

	FXAAPass *fxaaPass = new FXAAPass(m_window->get_extent(), totalImagesInFlight, m_settings.colorFormat, m_vignette);

	m_renderPipeline.push_renderpass(shadowPass);
	m_renderPipeline.push_renderpass(geometryPass);
	m_renderPipeline.push_renderpass(ssaoPass);
	m_renderPipeline.push_renderpass(ssaoBlurPass);
	m_renderPipeline.push_renderpass(compPass);
	m_renderPipeline.push_renderpass(forwardPass);
	m_renderPipeline.push_renderpass(fxaaPass);

	if (m_settings.renderingType == RendererType::TFORWARD)
		compPass->set_active(false);
	else
		forwardPass->set_active(false);

	if (m_settings.AAtype != AntialiasingType::FXAA)
		fxaaPass->set_active(false);

	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		pass->init(m_device);

		pass->create_framebuffer(m_device, m_memory, &m_swapchain);
	};

	m_deletionQueue.push_function([=]()
								  {
			for (RenderPass* pass : m_renderPipeline.renderpasses)
			{
				pass->cleanup(m_device, m_memory);
			} });
}

void Renderer::init_control_objects()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_frames[i].init(m_device, m_gpu, m_window->get_surface());
		m_deletionQueue.push_function([=]()
									  { m_frames[i].cleanup(m_device); });
	}

	m_uploadContext.init(m_device, m_gpu, m_window->get_surface());

	m_deletionQueue.push_function([=]()
								  { m_uploadContext.cleanup(m_device); });
}

void Renderer::init_descriptors()
{
	// Global descriptor manager setup

	m_descriptorMng.init(m_device);
	m_descriptorMng.create_pool(VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS, VK_MAX_OBJECTS);

	// GLOBAL SET
	VkDescriptorSetLayoutBinding camBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 0);
	VkDescriptorSetLayoutBinding sceneBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding shadowBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2); // ShadowMaps
	VkDescriptorSetLayoutBinding ssaoBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);	 // SSAO
	VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBufferBinding, shadowBinding, ssaoBinding};
	m_descriptorMng.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 4);

	// PER-OBJECT SET
	VkDescriptorSetLayoutBinding objectBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding materialBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding objectBindings[] = {objectBufferBinding, materialBufferBinding};
	m_descriptorMng.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, objectBindings, 2);

	// MATERIAL TEXTURE SET
	VkDescriptorSetLayoutBinding textureBinding1 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding textureBinding2 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding textureBinding3 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
	VkDescriptorSetLayoutBinding textureBinding4 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	VkDescriptorSetLayoutBinding textureBinding5 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
	VkDescriptorSetLayoutBinding textureBindings[] = {textureBinding1, textureBinding2, textureBinding3, textureBinding4, textureBinding5};
	m_descriptorMng.set_layout(DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, textureBindings, 5);

	// G BUFFER SET
	VkDescriptorSetLayoutBinding positionBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding normalBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding albedoBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
	VkDescriptorSetLayoutBinding materialBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	VkDescriptorSetLayoutBinding auxUniformBuffer = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
	VkDescriptorSetLayoutBinding gBindings[] = {positionBinding, normalBinding, albedoBinding, materialBinding, auxUniformBuffer};
	m_descriptorMng.set_layout(DescriptorLayoutType::G_BUFFER_LAYOUT, gBindings, 5);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		// Global Descriptor
		const size_t globalStrideSize = (utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu) + utils::pad_uniform_buffer_size(sizeof(SceneUniforms), m_gpu));
		m_frames[i].globalUniformBuffer.init(m_memory, globalStrideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)globalStrideSize);

		m_deletionQueue.push_function([=]()
									  { m_frames[i].globalUniformBuffer.cleanup(m_memory); });

		m_descriptorMng.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_frames[i].globalDescriptor);

		m_descriptorMng.set_descriptor_write(&m_frames[i].globalUniformBuffer, sizeof(CameraUniforms), 0,
											 &m_frames[i].globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);

		m_descriptorMng.set_descriptor_write(&m_frames[i].globalUniformBuffer, sizeof(SceneUniforms), utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu),
											 &m_frames[i].globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);

		// Object Descriptor
		const size_t objectStrideSize = (utils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu) + utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_gpu));
		m_frames[i].objectUniformBuffer.init(m_memory, VK_MAX_OBJECTS * objectStrideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)objectStrideSize);
		m_deletionQueue.push_function([=]()
									  { m_frames[i].objectUniformBuffer.cleanup(m_memory); });

		m_descriptorMng.allocate_descriptor_set(DescriptorLayoutType::OBJECT_LAYOUT, &m_frames[i].objectDescriptor);

		m_descriptorMng.set_descriptor_write(&m_frames[i].objectUniformBuffer, sizeof(ObjectUniforms), 0, &m_frames[i].objectDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);

		m_descriptorMng.set_descriptor_write(&m_frames[i].objectUniformBuffer, sizeof(MaterialUniforms), utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_gpu), &m_frames[i].objectDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);
	}

	m_deletionQueue.push_function([=]()
								  { m_descriptorMng.cleanup(); });

	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		pass->create_descriptors(m_device, m_gpu, m_memory, MAX_FRAMES_IN_FLIGHT);
	}
}

void Renderer::init_pipelines()
{
	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		pass->create_pipelines(m_device, m_descriptorMng);
	}
}

void Renderer::update_renderpasses()
{
	// GLFW update framebuffer
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window->get_window_obj(), &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window->get_window_obj(), &width, &height);
		glfwWaitEvents();
	}

	VK_CHECK(vkDeviceWaitIdle(m_device));

	// Swapchain recreation
	m_swapchain.cleanup(m_device, m_memory);
	m_swapchain.create(m_gpu, m_device, m_window->get_surface(), m_window->get_window_obj(), m_window->get_extent(), static_cast<uint32_t>(m_settings.bufferingType),
					   static_cast<VkFormat>(m_settings.colorFormat), static_cast<VkPresentModeKHR>(m_settings.screenSync));

	// Renderpass framebuffer updating
	size_t i = 0;
	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		if (pass->is_resizeable())
		{
			pass->set_extent(m_window->get_extent());
			pass->update(m_device, m_memory, &m_swapchain);
		}

		if (i == GEOMETRY)
			static_cast<SSAOPass *>(m_renderPipeline.renderpasses[SSAO])->set_g_buffer(pass->get_attachments()[0].image, pass->get_attachments()[1].image);
		if (i == SSAO)
			static_cast<SSAOBlurPass *>(m_renderPipeline.renderpasses[SSAO_BLUR])->set_ssao_buffer(pass->get_attachments()[0].image);
		i++;
	};

	static_cast<CompositionPass *>(m_renderPipeline.renderpasses[COMPOSITION])->set_g_buffer(m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[0].image, m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[1].image, m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[2].image, m_renderPipeline.renderpasses[GEOMETRY]->get_attachments()[3].image, m_descriptorMng);

	if (m_settings.AAtype == AntialiasingType::FXAA)
		static_cast<FXAAPass *>(m_renderPipeline.renderpasses[DefaultRenderPasses::FXAA])->set_output_buffer(m_settings.renderingType == RendererType::TDEFERRED ? m_renderPipeline.renderpasses[COMPOSITION]->get_attachments()[0].image : m_renderPipeline.renderpasses[FORWARD]->get_attachments()[0].image);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (m_settings.occlusionType == AmbientOcclusionType::SSAO)
			m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[SSAO_BLUR]->get_attachments().front().image.sampler,
												 m_renderPipeline.renderpasses[SSAO_BLUR]->get_attachments().front().image.view,
												 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_frames[i].globalDescriptor, 3);
		else
			m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[GEOMETRY]->get_attachments().front().image.sampler,
												 m_renderPipeline.renderpasses[GEOMETRY]->get_attachments().front().image.view,
												 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_frames[i].globalDescriptor, 3);
	}

	m_updateFramebuffers = false;
}

void Renderer::update_shadow_quality()

{
	VK_CHECK(vkDeviceWaitIdle(m_device));

	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;

	m_renderPipeline.renderpasses[SHADOW]->set_extent({SHADOW_RES, SHADOW_RES});
	m_renderPipeline.renderpasses[SHADOW]->update(m_device, m_memory);

	m_updateShadowQuality = false;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{

		m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[SHADOW]->get_attachments().front().image.sampler,
											 m_renderPipeline.renderpasses[SHADOW]->get_attachments().front().image.view,
											 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &m_frames[i].globalDescriptor, 2);
	}
}

void Renderer::update_context()
{

	if (!m_initialized)
		return;

	VK_CHECK(vkDeviceWaitIdle(m_device));

	// Cleanup
	m_deletionQueue.flush();

	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		pass->clean_framebuffer(m_device, m_memory);
	}
	m_swapchain.cleanup(m_device, m_memory);

	m_renderPipeline.renderpasses.clear();

	init_control_objects();

	// Recreation
	m_swapchain.create(m_gpu, m_device, m_window->get_surface(), m_window->get_window_obj(), m_window->get_extent(), static_cast<uint32_t>(m_settings.bufferingType),
					   static_cast<VkFormat>(m_settings.colorFormat), static_cast<VkPresentModeKHR>(m_settings.screenSync));

	init_renderpasses();

	init_descriptors();

	init_pipelines();

	init_resources();

	// if (m_settings.enableUI)
	// 	init_gui();

	Frame::guiEnabled = false;

	set_renderpass_resources();

	m_updateContext = false;
}
VULKAN_ENGINE_NAMESPACE_END