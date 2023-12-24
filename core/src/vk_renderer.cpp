
#include "engine/vk_renderer.h"

namespace vke
{
	void Renderer::run(Scene *const scene)
	{
		while (!glfwWindowShouldClose(m_window->get_window_obj()))
		{
			// I-O
			m_window->poll_events();
			render(scene);
		}
		shutdown();
	}

	void Renderer::shutdown()
	{
		VK_CHECK(vkDeviceWaitIdle(m_device));
		cleanup();
	}

	void Renderer::render(Scene *const scene)
	{
		// if(scene is dirty){
		// 	iterate scene tree and update mesh and lights vectors
		// 	dirty = false
		// }
		// for (auto mesh : scene->get_meshes())
		// {
		// 	for (size_t i = 0; i < mesh->get_num_geometries(); i++)
		// 	{
		// 		if (!mesh->get_geometry(i)->buffer_loaded)
		// 		{
		// 			upload_geometry_data(mesh->get_geometry(i));
		// 		}
		// 	}
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

		render_pass(m_frames[m_currentFrame].commandBuffer, imageIndex, scene);

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
			scene->get_active_camera()->set_projection(m_window->get_extent()->width, m_window->get_extent()->height);
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

		init_default_shaderpasses();

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

		// DEPTH STENCIL BUFFER SETUP

		VkExtent3D depthBufferExtent = {
			m_window->get_extent()->width,
			m_window->get_extent()->height,
			1};

		VmaAllocationCreateInfo dimg_allocinfo = {};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_depthBuffer.init(m_memory, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, dimg_allocinfo, depthBufferExtent);

		VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(m_depthBuffer.format, m_depthBuffer.image, VK_IMAGE_ASPECT_DEPTH_BIT);
		VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &m_depthView));

		m_deletionQueue.push_function([=]()
									  {
			vkDestroyImageView(m_device, m_depthView, nullptr);
			m_depthBuffer.cleanup(m_memory); });
	}

	void Renderer::init_default_renderpass()
	{
		// as in values it sould hace
		// multisampled number
		// stencil
		// depth

		VkAttachmentDescription depth_attachment = {};
		// Depth attachment
		depth_attachment.flags = 0;
		depth_attachment.format = m_depthBuffer.format;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency depthDependency{};
		depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depthDependency.dstSubpass = 0;
		depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		;
		depthDependency.srcAccessMask = 0;
		depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		;
		depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[2] = {colorAttachment, depth_attachment};
		VkSubpassDependency dependencies[2] = {dependency, depthDependency};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies;

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
			VkImageView attachments[2] = {
				m_swapchain.get_image_views()[i],
				m_depthView};

			fb_info.pAttachments = attachments;
			fb_info.attachmentCount = 2;

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

		VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();

		VK_CHECK(vkCreateFence(m_device, &uploadFenceCreateInfo, nullptr, &m_uploadContext.uploadFence));
		m_deletionQueue.push_function([=]()
									  { vkDestroyFence(m_device, m_uploadContext.uploadFence, nullptr); });

		VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(vkboot::find_queue_families(m_gpu, *m_window->get_surface()).graphicsFamily.value());
		VK_CHECK(vkCreateCommandPool(m_device, &uploadCommandPoolInfo, nullptr, &m_uploadContext.commandPool));

		m_deletionQueue.push_function([=]()
									  { vkDestroyCommandPool(m_device, m_uploadContext.commandPool, nullptr); });

		// allocate the default command buffer that we will use for the instant commands
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(m_uploadContext.commandPool, 1);

		VkCommandBuffer cmd;
		VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_uploadContext.commandBuffer));
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

	void Renderer::set_viewport(VkCommandBuffer commandBuffer)
	{
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
	}

	void Renderer::render_pass(VkCommandBuffer commandBuffer, uint32_t imageIndex, Scene *const scene)
	{
		VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_begin_info();

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		set_viewport(commandBuffer);

		VkRenderPassBeginInfo renderPassInfo = vkinit::renderpass_begin_info(m_renderPass, *m_window->get_extent(), m_framebuffers[imageIndex]);

		// CLEAR SETUP
		VkClearValue clearColor = {{{m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}}};
		VkClearValue clearDepth;
		clearDepth.depthStencil.depth = 1.f;
		VkClearValue clearValues[] = {clearColor, clearDepth};
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		upload_global_data(scene);

		draw_meshes(commandBuffer, scene->get_meshes());

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void Renderer::init_default_shaderpasses()
	{
		PipelineBuilder builder;
		builder.vertexInputInfo = vkinit::vertex_input_state_create_info();
		builder.inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions(true, false, false, false);

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

		builder.depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

		std::string shaderDir(VK_SHADER_DIR);

		m_shaderPasses["basic_unlit"] = new ShaderPass(shaderDir + "basic_unlit.glsl");
		m_shaderPasses["basic_phong"] = new ShaderPass(shaderDir + "basic_phong.glsl");

		for (auto pair : m_shaderPasses)
		{
			ShaderPass *pass = pair.second;

			pass->descriptorSetLayoutIDs.push_back(0);
			pass->descriptorSetLayoutIDs.push_back(1);

			auto shader = ShaderSource::read_file(pass->SHADER_FILE);

			if (shader.vertSource != "")
			{
				ShaderStage vertShaderStage = ShaderSource::create_shader_stage(m_device, VK_SHADER_STAGE_VERTEX_BIT, ShaderSource::compile_shader(shader.vertSource, shader.name + "vert", shaderc_vertex_shader, true));
				pass->stages.push_back(vertShaderStage);
			}
			if (shader.fragSource != "")
			{
				ShaderStage fragShaderStage = ShaderSource::create_shader_stage(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, ShaderSource::compile_shader(shader.fragSource, shader.name + "frag", shaderc_fragment_shader, true));
				pass->stages.push_back(fragShaderStage);
			}
			if (shader.geomSource != "")
			{
				ShaderStage geomShaderStage = ShaderSource::create_shader_stage(m_device, VK_SHADER_STAGE_GEOMETRY_BIT, ShaderSource::compile_shader(shader.geomSource, shader.name + "geom", shaderc_geometry_shader, true));
				pass->stages.push_back(geomShaderStage);
			}

			std::vector<VkDescriptorSetLayout> descriptorLayouts;
			for (auto &layoutID : pass->descriptorSetLayoutIDs)
			{
				descriptorLayouts.push_back(m_descriptorMng.get_layout(layoutID));
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkinit::pipeline_layout_create_info();
			pipelineLayoutInfo.setLayoutCount = descriptorLayouts.size();
			pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

			if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pass->pipelineLayout) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create pipeline layout!");
			}
			builder.pipelineLayout = pass->pipelineLayout;

			builder.shaderPass = pass;
			pass->pipeline = builder.build_pipeline(m_device, m_renderPass);

			m_deletionQueue.push_function([=]()
										  { pass->cleanup(m_device); });
		}
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

	void Renderer::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function)
	{
		VkCommandBuffer cmd = m_uploadContext.commandBuffer;

		VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

		function(cmd);

		VK_CHECK(vkEndCommandBuffer(cmd));

		VkSubmitInfo submit = vkinit::submit_info(&cmd);

		VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_uploadContext.uploadFence));

		vkWaitForFences(m_device, 1, &m_uploadContext.uploadFence, true, 9999999999);
		vkResetFences(m_device, 1, &m_uploadContext.uploadFence);

		vkResetCommandPool(m_device, m_uploadContext.commandPool, 0);
	}

	void Renderer::draw_meshes(VkCommandBuffer commandBuffer, const std::vector<Mesh *> meshes)
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
	}

	void Renderer::draw_mesh(VkCommandBuffer commandBuffer, Mesh *const m, int meshNum)
	{
		if (m->get_num_geometries() == 0)
			return;

		// Offset calculation
		uint32_t objectOffset = vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu) * meshNum;
		uint32_t globalOffset = m_globalUniformsBuffer.strideSize * m_currentFrame;
		uint32_t descriptorOffsets[] = {globalOffset, globalOffset, objectOffset};

		// ObjectUniforms objectData;
		ObjectUniforms objectData;
		objectData.model = m->get_model_matrix();
		objectData.color = static_cast<BasicPhongMaterial *>(m->get_material())->get_color();
		objectData.otherParams = {m->is_affected_by_fog(), false, false, false};
		m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory, &objectData, sizeof(ObjectUniforms), objectOffset);

		for (size_t i = 0; i < m->get_num_geometries(); i++)
		{
			Geometry *g = m->get_geometry(i);

			Material *mat = m->get_material(g->m_materialID);
			if (!mat)
			{
				// USE DEBUG MAT;
			}
			if (!mat->m_shaderPass)
			{
				mat->m_shaderPass = m_shaderPasses[mat->m_shaderPassID];
			}

			// TO DO: Upload material UBO !!!!

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->m_shaderPass->pipeline);

			VkDescriptorSet descriptors[] = {m_globalDescriptor.descriptorSet, m_frames[m_currentFrame].objectDescriptor.descriptorSet};
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->m_shaderPass->pipelineLayout, 0, 2, descriptors, 3, descriptorOffsets);
			// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipelineLayout, 0, 1, &m_globalDescriptor, 2, globalOffsets);
			// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipelineLayout, 1, 1, &m_frames[m_currentFrame].objectDescriptor, 1, &objectOffset);

			// BIND OBJECT BUFFERS
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
	}
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
		sceneParams.fogParams = {camera->get_near(), camera->get_far(), scene->get_fog_intensity(), 1.0f};
		sceneParams.fogColor = glm::vec4(scene->get_fog_color(), 1.0f);
		sceneParams.ambientColor = {0.5, 0.2, 0.7, 0.2};
		sceneParams.lightColor = {1.0, 1.0, 1.0, 0.0};
		sceneParams.lightPosition = {2.0, 2.0, 0.0, 0.0};

		m_globalUniformsBuffer.upload_data(m_memory, &sceneParams, sizeof(SceneUniforms),
										   m_globalUniformsBuffer.strideSize * m_currentFrame + vkutils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu));
	}
}

void vke::Renderer::upload_texture(Texture *const t)
{
	VkExtent3D extent = {t->m_width,
						 t->m_height,
						 t->m_depth};

	t->m_image->init(m_memory, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, extent);

	Buffer stagingBuffer;

	void *pixel_ptr = t->m_tmpCache;
	VkDeviceSize imageSize = t->m_width * t->m_height * t->m_depth * Image::BYTES_PER_PIXEL;

	stagingBuffer.init(m_memory, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	stagingBuffer.upload_data(m_memory, pixel_ptr, static_cast<size_t>(imageSize));

	free(t->m_tmpCache);

	// immediate_submit([=](VkCommandBuffer cmd)
	// 				 { t->m_image->upload_image(m_uploadContext.commandBuffer, &stagingBuffer) });

	m_deletionQueue.push_function([=]()
								  { t->m_image->cleanup(m_memory); });

	stagingBuffer.cleanup(m_memory);

	t->buffer_loaded = true;
}
