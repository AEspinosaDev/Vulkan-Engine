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

void Renderer::setup_renderpasses()
{

	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
	const uint32_t totalImagesInFlight = (uint32_t)m_settings.bufferingType + 1;

	ForwardPass *forwardPass = new ForwardPass(
		&m_context,
		m_window->get_extent(),
		totalImagesInFlight,
		m_settings.colorFormat,
		m_settings.depthFormat,
		m_settings.AAtype);

	ShadowPass *shadowPass = new ShadowPass(
		&m_context,
		{SHADOW_RES, SHADOW_RES},
		totalImagesInFlight,
		VK_MAX_LIGHTS,
		m_settings.depthFormat);

	m_renderPipeline.push_renderpass(shadowPass);
	m_renderPipeline.push_renderpass(forwardPass);
}



void Renderer::update_renderpasses()
{
	// GLFW update framebuffer
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window->get_handle(), &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window->get_handle(), &width, &height);
		glfwWaitEvents();
	}

	m_context.wait_for_device();

	m_context.recreate_swapchain(
		m_window->get_handle(),
		m_window->get_extent(),
		static_cast<uint32_t>(m_settings.bufferingType),
		static_cast<VkFormat>(m_settings.colorFormat),
		static_cast<VkPresentModeKHR>(m_settings.screenSync));

	// Renderpass framebuffer updating
	size_t i = 0;
	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		if (pass->resizeable())
		{
			pass->set_extent(m_window->get_extent());
			pass->update();
		}

		// set the resources of the previews pass

		i++;
	};

	m_updateFramebuffers = false;
}

// void Renderer::update_shadow_quality()

// {
// 	m_context.wait_for_device();

// 	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;

// 	m_renderPipeline.renderpasses[0]->set_extent({SHADOW_RES, SHADOW_RES});
// 	m_renderPipeline.renderpasses[0]->update();

// 	m_updateShadowQuality = false;

// 	for (size_t i = 0; i < m_context.frames.size(); i++)
// 	{

// 		// m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[0]->get_attachments().front().image.sampler,
// 		// 									 m_renderPipeline.renderpasses[0]->get_attachments().front().image.view,
// 		// 									 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &m_context.frames[i].globalDescriptor, 2);
// 	}
// }

VULKAN_ENGINE_NAMESPACE_END