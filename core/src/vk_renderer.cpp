
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
		// // It holds pools
		// m_descriptorAllocator = new DescriptorAllocator{};
		// m_descriptorAllocator->init(m_device);

		// // It holds descriptors
		// m_descriptorLayoutCache = new DescriptorLayoutCache{};
		// m_descriptorLayoutCache->init(m_device);

		// For textures in the future
		//  VkDescriptorSetLayoutBinding textureBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		//  VkDescriptorSetLayoutCreateInfo set3info = {};
		//  set3info.bindingCount = 1;
		//  set3info.flags = 0;
		//  set3info.pNext = nullptr;
		//  set3info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		//  set3info.pBindings = &textureBind;
		//  _singleTextureSetLayout = _descriptorLayoutCache->create_descriptor_layout(&set3info);

		// create a descriptor pool that will hold 10 uniform buffers
		std::vector<VkDescriptorPoolSize> sizes =
			{
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;
		pool_info.maxSets = 10;
		pool_info.poolSizeCount = (uint32_t)sizes.size();
		pool_info.pPoolSizes = sizes.data();

		VK_CHECK(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptorPool));

		// SET 0 GLOBAL
		VkDescriptorSetLayoutBinding camBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
		VkDescriptorSetLayoutBinding sceneBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBufferBinding};
		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.bindingCount = 2;
		setinfo.flags = 0;
		setinfo.pNext = nullptr;
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pBindings = bindings;

		VK_CHECK(vkCreateDescriptorSetLayout(m_device, &setinfo, nullptr, &m_globalSetLayout));

		// SET 1 PER-OBJECT
		VkDescriptorSetLayoutBinding objectBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 0);
		VkDescriptorSetLayoutCreateInfo set2info = {};
		set2info.bindingCount = 1;
		set2info.flags = 0;
		set2info.pNext = nullptr;
		set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set2info.pBindings = &objectBufferBinding;

		VK_CHECK(vkCreateDescriptorSetLayout(m_device, &set2info, nullptr, &m_objectSetLayout));

		const size_t sceneParamBufferSize = MAX_FRAMES_IN_FLIGHT * vkutils::pad_uniform_buffer_size(sizeof(SceneUniforms), m_gpu);
		create_buffer(&m_settings.sceneUniformBuffer, sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{

			// m_frames[i].descriptorAllocator = DescriptorAllocator{};
			// m_frames[i].dynamicDescriptorAllocator->init(m_device);

			// // 1 megabyte of dynamic data buffer
			// auto dynamicDataBuffer = create_buffer(1000000, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
			// m_frames[i].frameUniformBuffer.init(_allocator, dynamicDataBuffer, _gpuProperties.limits.minUniformBufferOffsetAlignment);

			create_buffer(&m_frames[i].cameraUniformBuffer, sizeof(CameraUniforms), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			create_buffer(&m_frames[i].objectUniformBuffer, 3 * vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

			// allocate one descriptor set for each frame
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_globalSetLayout;

			VK_CHECK(vkAllocateDescriptorSets(m_device, &allocInfo,
											  &m_frames[i].globalDescriptor));

			VkDescriptorSetAllocateInfo objectAllocInfo = {};
			objectAllocInfo.pNext = nullptr;
			objectAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			objectAllocInfo.descriptorPool = m_descriptorPool;
			objectAllocInfo.descriptorSetCount = 1;
			objectAllocInfo.pSetLayouts = &m_objectSetLayout;

			VK_CHECK(vkAllocateDescriptorSets(m_device, &objectAllocInfo, &m_frames[i].objectDescriptor));

			VkDescriptorBufferInfo cameraInfo;
			cameraInfo.buffer = m_frames[i].cameraUniformBuffer.buffer;
			cameraInfo.offset = 0;
			cameraInfo.range = sizeof(CameraUniforms);

			VkDescriptorBufferInfo sceneInfo;
			sceneInfo.buffer = m_settings.sceneUniformBuffer.buffer;
			sceneInfo.offset = 0;
			sceneInfo.range = sizeof(SceneUniforms);

			VkDescriptorBufferInfo objectInfo;
			objectInfo.buffer = m_frames[i].objectUniformBuffer.buffer;
			objectInfo.offset = 0;
			objectInfo.range = sizeof(ObjectUniforms);

			VkWriteDescriptorSet cameraWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_frames[i].globalDescriptor, &cameraInfo, 0);
			VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, m_frames[i].globalDescriptor, &sceneInfo, 1);
			VkWriteDescriptorSet objectWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, m_frames[i].objectDescriptor, &objectInfo, 0);

			VkWriteDescriptorSet setWrites[] = {cameraWrite, sceneWrite, objectWrite};

			vkUpdateDescriptorSets(m_device, 3, setWrites, 0, nullptr);
		}

		m_deletionQueue.push_function([=]()
									  {
										vkDestroyDescriptorSetLayout(m_device, m_globalSetLayout, nullptr); 
										vkDestroyDescriptorSetLayout(m_device, m_objectSetLayout, nullptr); 
										vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr); });
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

		uint32_t offset = vkutils::pad_uniform_buffer_size(sizeof(SceneUniforms), m_gpu) * m_currentFrame;

		upload_global_uniform_buffers(camera, &offset);

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
		VkDescriptorSetLayout setLayouts[] = {m_globalSetLayout, m_objectSetLayout};
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
		// Reorder by transparency
		uint32_t offset = vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu);
		std::vector<ObjectUniforms> matrixes;
		int i = 0;
		for (Mesh *m : meshes)
		{
			ObjectUniforms objectData;
			objectData.model = m->get_model_matrix();
			matrixes.push_back(objectData);
		}
		// m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory, matrixes.data(), sizeof(ObjectUniforms)*2 );
		// m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory, matrixes.data(), sizeof(ObjectUniforms)*matrixes.size(),{0});
		// m_settings.sceneUniformBuffer.upload_data(m_memory, &sceneParams, sizeof(SceneUniforms), offsets[0]);
		// m_settings.sceneUniformBuffer.upload_data(m_memory, &sceneParams, sizeof(SceneUniforms), offsets[0]);

		i = 0;
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
		if (!m->get_material() || !m->get_geometry() || !m->get_geometry()->loaded)
			return;

		Geometry *g = m->get_geometry();

		if (m->get_material()->m_pipelineAssigned == false)
		{
			init_pipeline(m->get_material());
		}

		// ObjectUniforms objectData;
		// objectData.model = m->get_model_matrix();
		// upload_buffer(&m_frames[m_currentFrame].objectUniformBuffer, &objectData, sizeof(ObjectUniforms));
		uint32_t offset2 = vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu) * meshNum;
		ObjectUniforms objectData;
		objectData.model = m->get_model_matrix();
		m_frames[m_currentFrame].objectUniformBuffer.upload_data(m_memory,&objectData, sizeof(ObjectUniforms),offset2);
		uint32_t offseets[2] = { vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu)*0, vkutils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu)*1 };
		// Like bind program
		// m_currentPipeline = m_selectedShader == 0 ? &m_pipelines["0"] : &m_pipelines["1"];
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipeline);
		uint32_t offset = vkutils::pad_uniform_buffer_size(sizeof(SceneUniforms), m_gpu) * m_currentFrame;
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipelineLayout, 0, 1, &m_frames[m_currentFrame].globalDescriptor, 1, &offset);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m->get_material()->m_pipelineLayout, 1, 1, &m_frames[m_currentFrame].objectDescriptor, 1, &offset2);

		if (!g->buffer_loaded)
			setup_geometry_buffers(g);

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

	void Renderer::create_buffer(Buffer *buffer, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;

		bufferInfo.size = allocSize;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = memoryUsage;

		VK_CHECK(vmaCreateBuffer(m_memory, &bufferInfo, &vmaallocInfo,
								 &buffer->buffer,
								 &buffer->allocation,
								 nullptr));

		m_deletionQueue.push_function([=]()
									  { vmaDestroyBuffer(m_memory, buffer->buffer,
														 buffer->allocation); });
	};

	void Renderer::setup_geometry_buffers(Geometry *g)
	{
		create_buffer(g->m_vbo, sizeof(g->m_vertexData[0]) * g->m_vertexData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		g->m_vbo->upload_data(m_memory, g->m_vertexData.data(), sizeof(g->m_vertexData[0]) * g->m_vertexData.size());
		if (g->indexed)
		{
			create_buffer(g->m_ibo, sizeof(g->m_vertexIndex[0]) * g->m_vertexIndex.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			g->m_ibo->upload_data(m_memory, g->m_vertexIndex.data(), sizeof(g->m_vertexIndex[0]) * g->m_vertexIndex.size());
		}
		g->buffer_loaded = true;
	}
	void Renderer::upload_global_uniform_buffers(Camera *camera, uint32_t *offsets)
	{
		// camera buffer
		if (camera->is_dirty())
			camera->set_projection(m_window->get_extent()->width, m_window->get_extent()->height);
		CameraUniforms camData;
		camData.view = camera->get_view();
		camData.proj = camera->get_projection();
		camData.viewProj = camera->get_projection() * camera->get_view();
		m_frames[m_currentFrame].cameraUniformBuffer.upload_data(m_memory, &camData, sizeof(CameraUniforms));
		// scene buffer
		// if (scene->is_dirty())
		{
			// scene.getParams turns clean
			SceneUniforms sceneParams;
			sceneParams.fogDistances = {1, 0, 0, 1};
			sceneParams.fogColor = {1, 0, 0, 1};
			sceneParams.ambientColor = {1, 1, 1, 1};
			m_settings.sceneUniformBuffer.upload_data(m_memory, &sceneParams, sizeof(SceneUniforms), offsets[0]);
		}
	}
}
