
#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::run(Scene *const scene)
{
	if (!m_initialized)
		init();

	while (!glfwWindowShouldClose(m_window->get_window_obj()))
	{
		// I-O
		Window::poll_events();

		render(scene);
	}
	shutdown();
}

void Renderer::shutdown()
{
	VK_CHECK(vkDeviceWaitIdle(m_device));
	on_shutdown();
}

void Renderer::on_before_render(Scene *const scene)
{
	if (!m_initialized)
		init();

	if (m_settings.enableUI && m_gui)
	{
		m_gui->render();
	}

	upload_global_data(scene);

	upload_object_data(scene);

	m_renderPipeline.renderpasses.back()->set_attachment_clear_value({m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a});
}

void Renderer::on_after_render(VkResult &renderResult, Scene *const scene)
{
	if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR || m_window->is_resized() || m_updateFramebuffers)
	{
		m_window->set_resized(false);
		update_renderpasses();
		scene->get_active_camera()->set_projection(m_window->get_extent().width, m_window->get_extent().height);

		for (Mesh *m : scene->get_meshes())
		{
			if (m) // If mesh exists
			{
				for (size_t i = 0; i < m->get_num_geometries(); i++)
				{
					Geometry *g = m->get_geometry(i);
					Material *mat = m->get_material(g->get_material_ID());
					if (!mat)
						return;
					// Update SSAO descriptor in materials
					m_descriptorMng.set_descriptor_write(m_renderPipeline.renderpasses[SSAO_BLUR]->get_attachments().front().image.sampler,
														 m_renderPipeline.renderpasses[SSAO_BLUR]->get_attachments().front().image.view,
														 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &mat->m_textureDescriptor, 6);
				}
			}
		}
	}
	else if (renderResult != VK_SUCCESS)
	{
		throw VKException("failed to present swap chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::render(Scene *const scene)
{

	on_before_render(scene);

	VK_CHECK(vkWaitForFences(m_device, 1, &m_frames[m_currentFrame].renderFence, VK_TRUE, UINT64_MAX));
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain.get_swapchain_obj(), UINT64_MAX, m_frames[m_currentFrame].presentSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		update_renderpasses();

		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw VKException("failed to acquire swap chain image!");
	}

	VK_CHECK(vkResetFences(m_device, 1, &m_frames[m_currentFrame].renderFence));
	VK_CHECK(vkResetCommandBuffer(m_frames[m_currentFrame].commandBuffer, 0));

	VkCommandBufferBeginInfo beginInfo = init::command_buffer_begin_info();

	if (vkBeginCommandBuffer(m_frames[m_currentFrame].commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw VKException("failed to begin recording command buffer!");
	}
	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		if (pass->is_active())
			pass->render(m_frames[m_currentFrame], m_currentFrame, scene, imageIndex);
	}
	if (vkEndCommandBuffer(m_frames[m_currentFrame].commandBuffer) != VK_SUCCESS)
	{
		throw VKException("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo = init::submit_info(&m_frames[m_currentFrame].commandBuffer);
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = {m_frames[m_currentFrame].presentSemaphore};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	VkSemaphore signalSemaphores[] = {m_frames[m_currentFrame].renderSemaphore};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frames[m_currentFrame].renderFence) != VK_SUCCESS)
	{
		throw VKException("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = init::present_info();
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = {m_swapchain.get_swapchain_obj()};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	on_after_render(result, scene);
}
void Renderer::on_awake()
{
	m_frames.resize(MAX_FRAMES_IN_FLIGHT);

	// Creating default renderpasses
	ForwardPass *forwardPass = new ForwardPass(m_window->get_extent(),
											   (uint32_t)m_settings.bufferingType + 1,
											   m_settings.colorFormat,
											   m_settings.depthFormat,
											   (VkSampleCountFlagBits)m_settings.AAtype);

	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
	ShadowPass *shadowPass = new ShadowPass({SHADOW_RES, SHADOW_RES}, VK_MAX_LIGHTS, m_settings.depthFormat);

	GeometryPass *geometryPass = new GeometryPass(m_window->get_extent(),
												  (uint32_t)m_settings.bufferingType + 1,

												  m_settings.depthFormat);

	Mesh *vignette = Mesh::create_quad();
	m_deletionQueue.push_function([=]()
								  { vignette->get_geometry()->cleanup(m_memory); });

	SSAOPass *ssaoPass = new SSAOPass(m_window->get_extent(), vignette);
	SSAOBlurPass *ssaoBlurPass = new SSAOBlurPass(m_window->get_extent(), vignette);

	m_renderPipeline.push_renderpass(shadowPass);
	m_renderPipeline.push_renderpass(geometryPass);
	m_renderPipeline.push_renderpass(ssaoPass);
	m_renderPipeline.push_renderpass(ssaoBlurPass);
	m_renderPipeline.push_renderpass(forwardPass);
}

void Renderer::on_init()
{

	// BOOT Vulkan
	boot::VulkanBooter booter(m_enableValidationLayers);

	// Create instance
	m_instance = booter.boot_vulkan();
	m_debugMessenger = booter.create_debug_messenger(m_instance);

	// Create window and surface
	if (!m_window->is_initialized())
		m_window->init();

	create_surface(m_instance, m_window);
	// m_window->create_surface(m_instance);

	// Get gpu
	m_gpu = booter.pick_graphics_card_device(m_instance, m_window->get_surface());

	// Create logical device
	m_device = booter.create_logical_device(
		m_graphicsQueue,
		m_presentQueue,
		m_gpu,
		utils::get_gpu_features(m_gpu),
		m_window->get_surface());

	// Setup VMA
	m_memory = booter.setup_memory(m_instance, m_device, m_gpu);

	// Create swapchain
	m_swapchain.create(m_gpu, m_device, m_window->get_surface(), m_window->get_window_obj(), m_window->get_extent(), static_cast<uint32_t>(m_settings.bufferingType),
					   static_cast<VkFormat>(m_settings.colorFormat), static_cast<VkPresentModeKHR>(m_settings.screenSync));

	init_control_objects();

	init_renderpasses();

	init_descriptors();

	init_pipelines();

	init_resources();

	if (m_settings.enableUI)
		init_gui();
}

void Renderer::on_shutdown()
{
	if (m_initialized)
	{
		m_deletionQueue.flush();

		// Destroy passes framebuffers and resources
		for (RenderPass *pass : m_renderPipeline.renderpasses)
		{
			pass->clean_framebuffer(m_device, m_memory);
		}

		m_swapchain.cleanup(m_device, m_memory);

		vmaDestroyAllocator(m_memory);

		vkDestroyDevice(m_device, nullptr);

		if (m_enableValidationLayers)
		{
			utils::destroy_debug_utils_messenger_EXT(m_instance, m_debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(m_instance, m_window->get_surface(), nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	m_window->destroy();

	glfwTerminate();
}

void Renderer::init_gui()
{
	if (m_gui)
	{
		m_gui->init(m_instance, m_device, m_gpu, m_graphicsQueue, m_renderPipeline.renderpasses.back()->get_obj(), m_swapchain.get_image_format(), (VkSampleCountFlagBits)m_settings.AAtype, m_window->get_window_obj());
		m_deletionQueue.push_function([=]()
									  { m_gui->cleanup(m_device); });
	}
}
VULKAN_ENGINE_NAMESPACE_END