
#include <engine/core/renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::run(Scene *const scene)
{
	while (!glfwWindowShouldClose(m_window->get_handle()))
	{
		// I-O
		Window::poll_events();
		render(scene);
	}
	shutdown(scene);
}

void Renderer::shutdown(Scene *const scene)
{
	m_context.wait_for_device();
	on_shutdown(scene);
}

void Renderer::on_before_render(Scene *const scene)
{

	if (Frame::guiEnabled && m_gui)
		m_gui->render();

	upload_global_data(scene);

	upload_object_data(scene);

	m_renderPipeline.renderpasses[1]->set_attachment_clear_value(
		{m_settings.clearColor.r,
		 m_settings.clearColor.g,
		 m_settings.clearColor.b,
		 m_settings.clearColor.a});

}

void Renderer::on_after_render(VkResult &renderResult, Scene *const scene)
{
	if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR || m_window->is_resized() || m_updateFramebuffers)
	{
		m_window->set_resized(false);
		update_renderpasses();
		scene->get_active_camera()->set_projection(m_window->get_extent().width, m_window->get_extent().height);
	}
	else if (renderResult != VK_SUCCESS)
	{
		throw VKException("failed to present swap chain image!");
	}
	if (m_updateShadowQuality)
		update_shadow_quality();

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::render(Scene *const scene)
{
	if (!m_initialized)
		init();

	uint32_t imageIndex;
	VkResult imageResult = m_context.aquire_present_image(m_frames[m_currentFrame], imageIndex);

	if (imageResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		update_renderpasses();
		return;
	}
	else if (imageResult != VK_SUCCESS && imageResult != VK_SUBOPTIMAL_KHR)
	{
		throw VKException("failed to acquire swap chain image!");
	}

	on_before_render(scene);

	m_context.begin_command_buffer(m_frames[m_currentFrame]);

	for (RenderPass *pass : m_renderPipeline.renderpasses)
	{
		if (pass->is_active())
			pass->render(m_frames[m_currentFrame], m_currentFrame, scene, imageIndex);
	}

	m_context.end_command_buffer(m_frames[m_currentFrame]);

	VkResult renderResult = m_context.present_image(m_frames[m_currentFrame], imageIndex);

	on_after_render(renderResult, scene);
}
void Renderer::on_awake()
{
	MAX_FRAMES_IN_FLIGHT = static_cast<int>(m_settings.bufferingType);
	m_frames.resize(MAX_FRAMES_IN_FLIGHT);
	m_vignette = Mesh::create_quad();
}

void Renderer::on_init()
{
	if (!m_window->initialized())
		m_window->init();

	m_context.init(
		m_window->get_handle(),
		m_window->get_extent(),
		static_cast<uint32_t>(m_settings.bufferingType),
		static_cast<VkFormat>(m_settings.colorFormat),
		static_cast<VkPresentModeKHR>(m_settings.screenSync));

	init_control_objects();

	init_renderpasses();

	init_descriptors();

	init_pipelines();

	init_resources();

	if (m_settings.enableUI)
		init_gui();

	Frame::guiEnabled = m_settings.enableUI;
}

void Renderer::on_shutdown(Scene *const scene)
{
	if (m_initialized)
	{
		m_deletionQueue.flush();

		m_vignette->get_geometry()->get_render_data().vbo.cleanup(m_context.memory);
		m_vignette->get_geometry()->get_render_data().ibo.cleanup(m_context.memory);
		Texture::DEBUG_TEXTURE->m_image.cleanup(m_context.device, m_context.memory);

		if (scene)
			for (Mesh *m : scene->get_meshes())
			{
				for (size_t i = 0; i < m->get_num_geometries(); i++)
				{
					Geometry *g = m->get_geometry(i);
					RenderData rd = g->get_render_data();
					if (rd.loaded)
					{
						rd.vbo.cleanup(m_context.memory);
						if (g->indexed())
							rd.ibo.cleanup(m_context.memory);

						rd.loaded = false;
						g->set_render_data(rd);
					}
				}
			}

		for (RenderPass *pass : m_renderPipeline.renderpasses)
		{
			pass->clean_framebuffer();
		}

		m_context.cleanup();
	}

	m_window->destroy();

	glfwTerminate();
}

void Renderer::init_gui()
{
	if (m_gui)
	{
		m_gui->init(m_context.instance,
					m_context.device,
					m_context.gpu,
					m_context.graphicsQueue,
					m_renderPipeline.renderpasses[1]->get_handle(),
					m_context.swapchain.get_image_format(),
					(VkSampleCountFlagBits)m_settings.AAtype,
					m_window->get_handle());
		m_deletionQueue.push_function([=]()
									  { m_gui->cleanup(m_context.device); });
	}
}
VULKAN_ENGINE_NAMESPACE_END