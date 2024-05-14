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
	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		pass->init(m_device);

		if (pass->is_default()) // Check if its default in order to get swapchain image views
			pass->create_framebuffer(m_device, m_memory, &m_swapchain);
		else
		{
			pass->create_framebuffer(m_device, m_memory);
		}
	};

	m_deletionQueue.push_function([=]()
								  { 	
									for (RenderPass *pass : m_renderPipeline.renderpasses)
		{
			pass->cleanup(m_device,m_memory);
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
	m_descriptorMng.init(m_device);
	m_descriptorMng.create_pool(10, 10, 10, 20, 10);

	// GLOBAL SET
	VkDescriptorSetLayoutBinding camBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 0);
	VkDescriptorSetLayoutBinding sceneBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBufferBinding};
	m_descriptorMng.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 2);

	// PER-OBJECT SET
	VkDescriptorSetLayoutBinding objectBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	VkDescriptorSetLayoutBinding materialBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding objectBindings[] = {objectBufferBinding, materialBufferBinding};
	m_descriptorMng.set_layout(DescriptorLayoutType::OBJECT_LAYOUT, objectBindings, 2);

	// TEXTURE SET
	VkDescriptorSetLayoutBinding textureBinding0 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0); // ShadowMap
	VkDescriptorSetLayoutBinding textureBinding1 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutBinding textureBinding2 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
	VkDescriptorSetLayoutBinding textureBinding3 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	VkDescriptorSetLayoutBinding textureBinding4 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
	VkDescriptorSetLayoutBinding textureBinding5 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 5);
	VkDescriptorSetLayoutBinding textureBinding6 = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6); // OcclusionMap
	VkDescriptorSetLayoutBinding textureBindings[] = {textureBinding0, textureBinding1, textureBinding2, textureBinding3, textureBinding4, textureBinding5, textureBinding6};
	m_descriptorMng.set_layout(DescriptorLayoutType::TEXTURE_LAYOUT, textureBindings, 7);

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
			static_cast<SSAOPass *>(m_renderPipeline.renderpasses[SSAO])->set_geometry_buffer(pass->get_attachments()[0].image, pass->get_attachments()[1].image, pass->get_attachments()[2].image);
		if (i == SSAO)
			static_cast<SSAOBlurPass *>(m_renderPipeline.renderpasses[SSAO_BLUR])->set_ssao_buffer(pass->get_attachments()[0].image);
		i++;
	};
}

VULKAN_ENGINE_NAMESPACE_END