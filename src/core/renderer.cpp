
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

	m_pipeline.renderpasses[1]->set_attachment_clear_value({m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a});
}

void Renderer::on_after_render(VkResult &renderResult, Scene *const scene)
{
	if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR || m_window->is_resized() || m_updateFramebuffers)
	{
		m_window->set_resized(false);
		update_renderpasses();
		scene->get_active_camera()->set_projection(m_window->get_extent()->width, m_window->get_extent()->height);
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
	VK_CHECK(vkResetCommandBuffer(m_frames[m_currentFrame].commandBuffer, /*VkCommandBufferResetFlagBits*/ 0));

	VkCommandBufferBeginInfo beginInfo = init::command_buffer_begin_info();

	if (vkBeginCommandBuffer(m_frames[m_currentFrame].commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw VKException("failed to begin recording command buffer!");
	}

	for (RenderPass *pass : m_pipeline.renderpasses)
	{
		if (pass->is_active())
			pass->render(m_frames[m_currentFrame], m_currentFrame, scene, imageIndex);
	}

	if (vkEndCommandBuffer(m_frames[m_currentFrame].commandBuffer) != VK_SUCCESS)
	{
		throw VKException("failed to record command buffer!");
	}

	// prepare the submission to the queue.
	// we want to wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready
	// we will signal the renderSemaphore, to signal that rendering has finished
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

	// this will put the image we just rendered to into the visible window.
	// we want to wait on the renderSemaphore for that,
	// as its necessary that drawing commands have finished before the image is displayed to the user
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
	ForwardPass *forwardPass = new ForwardPass(*m_window->get_extent(),
											   (uint32_t)m_settings.bufferingType + 1,
											   m_settings.colorFormat,
											   m_settings.depthFormat,
											   (VkSampleCountFlagBits)m_settings.AAtype);

	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
	ShadowPass *shadowPass = new ShadowPass({SHADOW_RES, SHADOW_RES}, VK_MAX_LIGHTS, m_settings.depthFormat);

	GUIPass *guiPass = new GUIPass(*m_window->get_extent(),
								   (uint32_t)m_settings.bufferingType + 1,
								   m_settings.colorFormat,
								   m_settings.depthFormat,
								   (VkSampleCountFlagBits)m_settings.AAtype, m_gui);

	m_pipeline.push_renderpass(shadowPass);
	m_pipeline.push_renderpass(forwardPass);
}

void Renderer::on_init()
{
	if (!m_window->m_initialized)
		m_window->init();

	// BOOT Vulkan
	boot::VulkanBooter booter(&m_instance,
							  &m_debugMessenger,
							  &m_gpu,
							  &m_device,
							  &m_graphicsQueue, m_window->get_surface(),
							  &m_presentQueue, &m_memory, m_enableValidationLayers);
	booter.boot_vulkan();

	// Create surface
	VK_CHECK(glfwCreateWindowSurface(m_instance, m_window->get_window_obj(), nullptr, m_window->get_surface()));

	// Create and setup devices and memory allocation
	booter.pick_graphics_card_device();
	booter.create_logical_device(utils::get_gpu_features(m_gpu));
	booter.setup_memory();

	// Create swapchain
	m_swapchain.create(m_gpu, m_device, *m_window->get_surface(), m_window->get_window_obj(), *m_window->get_extent(), static_cast<uint32_t>(m_settings.bufferingType),
					   static_cast<VkFormat>(m_settings.colorFormat), static_cast<VkPresentModeKHR>(m_settings.screenSync));

	init_renderpasses();

	init_control_objects();

	init_descriptors();

	init_shaderpasses();

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
		for (RenderPass *pass : m_pipeline.renderpasses)
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
		vkDestroySurfaceKHR(m_instance, *m_window->get_surface(), nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	m_window->destroy();

	glfwTerminate();
}

void Renderer::init_gui()
{
	if (m_gui)
	{
		m_gui->init(m_instance, m_device, m_gpu, m_graphicsQueue, m_pipeline.renderpasses[1]->get_obj(), m_swapchain.get_image_format(), (VkSampleCountFlagBits)m_settings.AAtype, m_window->get_window_obj());
		m_deletionQueue.push_function([=]()
									  { m_gui->cleanup(m_device); });
	}
}
VULKAN_ENGINE_NAMESPACE_END