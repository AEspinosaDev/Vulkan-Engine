
#include "engine/vk_renderer.h"

namespace vke
{
	void Renderer::run(std::vector<Mesh *> meshes, Camera *camera)
	{
		while (!glfwWindowShouldClose(m_window->get_window_obj()))
		{
			// I-O
			m_window->poll_events();
			render(meshes, camera);
		}
		shutdown();
	}

	void Renderer::shutdown()
	{
		VK_CHECK(vkDeviceWaitIdle(m_device));
		cleanup();
	}

	void Renderer::render(std::vector<Mesh *> meshes, Camera *camera)
	{
		// if(scene is dirty){
		// 	iterate scene tree and update mesh and lights vectors
		// 	dirty = false
		// }

		VK_CHECK(vkWaitForFences(m_device, 1, &m_frames[m_currentFrame].renderFence, VK_TRUE, UINT64_MAX));
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_device, *m_swapchain.get_swapchain_obj(), UINT64_MAX, m_frames[m_currentFrame].presentSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swap_chain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		VK_CHECK(vkResetFences(m_device, 1, &m_frames[m_currentFrame].renderFence));
		VK_CHECK(vkResetCommandBuffer(m_frames[m_currentFrame].commandBuffer, /*VkCommandBufferResetFlagBits*/ 0));

		render_pass(m_frames[m_currentFrame].commandBuffer, imageIndex, meshes, camera);

		// prepare the submission to the queue.
		// we want to wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready
		// we will signal the renderSemaphore, to signal that rendering has finished
		VkSubmitInfo submitInfo = vkinit::submit_info(&m_frames[m_currentFrame].commandBuffer);
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
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// this will put the image we just rendered to into the visible window.
		// we want to wait on the renderSemaphore for that,
		// as its necessary that drawing commands have finished before the image is displayed to the user
		VkPresentInfoKHR presentInfo = vkinit::present_info();
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = {*m_swapchain.get_swapchain_obj()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->is_resized())
		{
			m_window->set_resized(false);
			recreate_swap_chain();
			camera->set_projection(m_window->get_extent()->width, m_window->get_extent()->height);
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::init_vulkan()
	{

		vkboot::VulkanBooter booter(&m_instance,
									&m_debugMessenger,
									&m_gpu,
									&m_device,
									&m_graphicsQueue, m_window->get_surface(),
									&m_presentQueue, &m_memory, &m_enableValidationLayers);

		booter.boot_vulkan();

		VK_CHECK(glfwCreateWindowSurface(m_instance, m_window->get_window_obj(), nullptr, m_window->get_surface()));

		booter.setup_devices();

		booter.setup_memory();

		create_swapchain();

		init_default_renderpass();

		init_framebuffers();

		init_control_objects();

		init_descriptors();

		// init_pipelines();

		m_initialized = true;
	}

	void Renderer::cleanup()
	{
		if (m_initialized)
		{
			m_deletionQueue.flush();

			vkDestroyDevice(m_device, nullptr);

			if (m_enableValidationLayers)
			{
				vkutils::destroy_debug_utils_messenger_EXT(m_instance, m_debugMessenger, nullptr);
			}
			vkDestroySurfaceKHR(m_instance, *m_window->get_surface(), nullptr);
			vkDestroyInstance(m_instance, nullptr);
		}

		m_window->destroy();

		glfwTerminate();
	}

	void Renderer::create_swapchain()
	{
		m_swapchain.create(&m_gpu, &m_device, m_window->get_surface(), m_window->get_window_obj(), m_window->get_extent());

		m_deletionQueue.push_function([=]()
									  {
										  // vkDestroySwapchainKHR(_device, _swapchain, nullptr);
										  cleanup_swap_chain(); });
	}

	void Renderer::init_default_renderpass()
	{
		// as in values it sould hace
		// multisampled number
		// stencil
		// depth

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = *m_swapchain.get_image_format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}

		m_deletionQueue.push_function([=]()
									  { vkDestroyRenderPass(m_device, m_renderPass, nullptr); });
	}

	void Renderer::init_framebuffers()
	{
		auto size = m_swapchain.get_image_views().size();

		m_framebuffers.resize(size);

		VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(m_renderPass, *m_window->get_extent());
		for (size_t i = 0; i < size; i++)
		{
			VkImageView attachments[] = {
				m_swapchain.get_image_views()[i]};

			fb_info.pAttachments = attachments;

			if (vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void Renderer::init_control_objects()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_frames[i].init(m_device, m_gpu, *m_window->get_surface());
			m_deletionQueue.push_function([=]()
										  { m_frames[i].cleanup(m_device); });
		}
	}

	void Renderer::init_descriptors()
	{

		m_descriptorMng.init(m_device);
		m_descriptorMng.create_pool(10, 10, 10, 10);

		// GLOBAL
		VkDescriptorSetLayoutBinding camBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 0);
		VkDescriptorSetLayoutBinding sceneBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBufferBinding};
		m_descriptorMng.set_layout(0, bindings, 2);

		// PER-OBJECT
		VkDescriptorSetLayoutBinding objectBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 0);
		m_descriptorMng.set_layout(1, &objectBufferBinding, 1);

		const size_t strideSize = (vkutils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu) + vkutils::pad_uniform_buffer_size(sizeof(SceneUniforms), m_gpu));
		const size_t globalUBOSize = MAX_FRAMES_IN_FLIGHT * strideSize;
		m_globalUniformsBuffer.init(m_memory, globalUBOSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, strideSize);
		m_deletionQueue.push_function([=]()
									  { m_globalUniformsBuffer.cleanup(m_memory); });

		m_descriptorMng.allocate_descriptor_set(0, &m_globalDescriptor);

		m_descriptorMng.set_descriptor_write(&m_globalUniformsBuffer, sizeof(CameraUniforms), 0,
											 &m_globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);

		m_descriptorMng.set_descriptor_write(&m_globalUniformsBuffer, sizeof(SceneUniforms), vkutils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu),
											 &m_globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{

			const size_t strideSize = vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu);
			m_frames[i].objectUniformBuffer.init(m_memory, MAX_OBJECTS_IN_FLIGHT * strideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, strideSize);
			m_deletionQueue.push_function([=]()
										  { m_frames[i].objectUniformBuffer.cleanup(m_memory); });

			m_descriptorMng.allocate_descriptor_set(1, &m_frames[i].objectDescriptor);

			m_descriptorMng.set_descriptor_write(&m_frames[i].objectUniformBuffer, sizeof(ObjectUniforms), 0, &m_frames[i].objectDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);
		}

		m_deletionQueue.push_function([=]()
									  { m_descriptorMng.cleanup(); });
	}

	void Renderer::render_pass(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::vector<Mesh *> meshes, Camera *camera)
	{
		VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_begin_info();
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		VkRenderPassBeginInfo renderPassInfo = vkinit::renderpass_begin_info(m_renderPass, *m_window->get_extent(), m_framebuffers[imageIndex]);

		// Viewport setup
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_window->get_extent()->width;
		viewport.height = (float)m_window->get_extent()->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = *m_window->get_extent();
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Clear COLOR | DEPTH | STENCIL ??
		VkClearValue clearColor = {{{m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		upload_global_data(camera);

		draw_meshes(commandBuffer, meshes);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void Renderer::init_pipeline(Material *m)
	{

		PipelineBuilder builder;
		builder.vertexInputInfo = vkinit::vertex_input_state_create_info();
		builder.inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		builder.vertexInputInfo.vertexBindingDescriptionCount = 1;
		builder.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		builder.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		builder.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		builder.viewport.x = 0.0f;
		builder.viewport.y = 0.0f;
		builder.viewport.width = (float)m_window->get_extent()->width;
		builder.viewport.height = (float)m_window->get_extent()->height;
		builder.viewport.minDepth = 0.0f;
		builder.viewport.maxDepth = 1.0f;
		builder.scissor.offset = {0, 0};
		builder.scissor.extent = *m_window->get_extent();

		builder.rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

		builder.multisampling = vkinit::multisampling_state_create_info();

		builder.colorBlendAttachment = vkinit::color_blend_attachment_state();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkinit::pipeline_layout_create_info();
		// hook the global set layout
		pipelineLayoutInfo.setLayoutCount = 2;
		VkDescriptorSetLayout setLayouts[] = {m_descriptorMng.get_layout(0), m_descriptorMng.get_layout(1)};
		pipelineLayoutInfo.pSetLayouts = setLayouts;

		// VkPipelineCache

		if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m->m_pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

		builder.pipelineLayout = m->m_pipelineLayout;

		// Compile shaders
		std::string shaderDir(SHADER_DIR);
		// Populate shader list
		auto shader = Shader::read_file(shaderDir + "test.glsl");

		VkShaderModule vertShaderModule{}, fragShaderModule{}, geomShaderModule{}, tessShaderModule{};
		if (shader.vertSource != "")
		{
			vertShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.vertSource, shader.name + "vert", shaderc_vertex_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule));
		}
		if (shader.fragSource != "")
		{
			fragShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.fragSource, shader.name + "frag", shaderc_fragment_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule));
		}
		if (shader.geomSource != "")
		{
			geomShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.geomSource, shader.name + "geom", shaderc_geometry_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_GEOMETRY_BIT, geomShaderModule));
		}
		if (shader.tessSource != "")
		{
			tessShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.tessSource, shader.name + "tess", shaderc_tess_control_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_GEOMETRY_BIT, tessShaderModule));
		}

		m->m_pipeline = builder.build_pipeline(m_device, m_renderPass);

		vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
		vkDestroyShaderModule(m_device, geomShaderModule, nullptr);
		vkDestroyShaderModule(m_device, tessShaderModule, nullptr);

		builder.shaderStages.clear();
		m->m_pipelineAssigned = true;

		// m_deletionQueue.push_function([=]()
		// 							  {
		// 	for (auto &pipeline : m_pipelines)
		// 	{
		// 		vkDestroyPipeline(m_device, pipeline.second, nullptr);
		// 	}
		// 	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr); });
	}

	void Renderer::recreate_swap_chain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_window->get_window_obj(), &width, &height);

		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_window->get_window_obj(), &width, &height);
			glfwWaitEvents();
		}

		VK_CHECK(vkDeviceWaitIdle(m_device));
		cleanup_swap_chain();
		create_swapchain();
		init_framebuffers();
	}

	void Renderer::cleanup_swap_chain()
	{
		for (size_t i = 0; i < m_framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
		}

		m_swapchain.cleanup(&m_device);
	}

	void Renderer::draw_meshes(VkCommandBuffer commandBuffer, std::vector<Mesh *> meshes)
	{

		int i = 0;
		for (Mesh *m : meshes)
		{
			if (m)
			{
				draw_mesh(commandBuffer, m, i);
			}
			i++;
		}
		// Draw helpers ...
	}

	void Renderer::draw_mesh(VkCommandBuffer commandBuffer, Mesh *m, int meshNum)
	{
		if (!m->get_geometry() || !m->get_geometry()->loaded)
			return;

		if (!m->get_material())
			// bind default material
			return;

		if (m->get_material()->m_pipelineAssigned == false)
		{
			init_pipeline(m->get_material());
		}

		// Offset calculation
		uint32_t objectOffset = vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu) * meshNum;
		uint32_t globalOffset = m_globalUniformsBuffer.strideSize * m_currentFrame;
		// uint32_t globalOffsets[2] = {globalOffset, globalOffset};
		uint32_t descriptorOffsets[] = {globalOffset, globalOffset, objectOffset};

		// ObjectUniforms objectData;
		ObjectUniforms objectData;
		objectData.model = m->get_model_matrix();
		m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory, &objectData, sizeof(ObjectUniforms), objectOffset);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipeline);

		VkDescriptorSet descriptors[] = {m_globalDescriptor.descriptorSet, m_frames[m_currentFrame].objectDescriptor.descriptorSet};
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipelineLayout, 0, 2, descriptors, 3, descriptorOffsets);
		// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipelineLayout, 0, 1, &m_globalDescriptor, 2, globalOffsets);
		// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipelineLayout, 1, 1, &m_frames[m_currentFrame].objectDescriptor, 1, &objectOffset);

		// BIND OBJECT BUFFERS
		Geometry *g = m->get_geometry();

		if (!g->buffer_loaded)
			upload_geometry_data(g);

		VkBuffer vertexBuffers[] = {g->m_vbo->buffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		if (g->indexed)
		{
			vkCmdBindIndexBuffer(commandBuffer, g->m_ibo->buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(g->m_vertexIndex.size()), 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, static_cast<uint32_t>(g->m_vertexData.size()), 1, 0, 0);
		}
	}

	void Renderer::upload_geometry_data(Geometry *g)
	{
		g->m_vbo->init(m_memory, sizeof(g->m_vertexData[0]) * g->m_vertexData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		g->m_vbo->upload_data(m_memory, g->m_vertexData.data(), sizeof(g->m_vertexData[0]) * g->m_vertexData.size());
		m_deletionQueue.push_function([=]()
									  { g->m_vbo->cleanup(m_memory); });
		if (g->indexed)
		{
			g->m_ibo->init(m_memory, sizeof(g->m_vertexIndex[0]) * g->m_vertexIndex.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			g->m_ibo->upload_data(m_memory, g->m_vertexIndex.data(), sizeof(g->m_vertexIndex[0]) * g->m_vertexIndex.size());
			m_deletionQueue.push_function([=]()
										  { g->m_ibo->cleanup(m_memory); });
		}
		g->buffer_loaded = true;
	}
	void Renderer::upload_global_data(Camera *camera)
	{

		if (camera->is_dirty())
			camera->set_projection(m_window->get_extent()->width, m_window->get_extent()->height);
		CameraUniforms camData;
		camData.view = camera->get_view();
		camData.proj = camera->get_projection();
		camData.viewProj = camera->get_projection() * camera->get_view();

		// m_cameraUniformBuffer.upload_data(m_memory, &camData, sizeof(CameraUniforms),
		// 								  vkutils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu) * m_currentFrame);
		m_globalUniformsBuffer.upload_data(m_memory, &camData, sizeof(CameraUniforms),
										   m_globalUniformsBuffer.strideSize * m_currentFrame);

		// if (scene->is_dirty())
		{
			// scene.getParams turns clean
			SceneUniforms sceneParams;
			sceneParams.fogParams = {.1f, 100.0f, 2.5f, 1};
			sceneParams.fogColor = {0.75, 0.75, 0.75, 1};
			sceneParams.ambientColor = {0.5, 0.2, 0.7, 1};

			m_globalUniformsBuffer.upload_data(m_memory, &sceneParams, sizeof(SceneUniforms),
											   m_globalUniformsBuffer.strideSize * m_currentFrame + vkutils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu));
		}
	}
}
