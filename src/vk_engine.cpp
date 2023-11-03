#include "vk_engine.h"





void VulkanEngine::init_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_window = glfwCreateWindow(m_globalParams.width, m_globalParams.height, "Vulkan Renderer", nullptr, nullptr);

	//WINDOW CALLBACKS
	glfwSetWindowUserPointer(m_window, this);

	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* w, int width, int heigth)
		{
			static_cast<VulkanEngine*>(glfwGetWindowUserPointer(w))->window_resize_callback(w, width, heigth);
		});
	glfwSetKeyCallback(m_window, [](GLFWwindow* w, int key, int scancode, int action, int mods)
		{
			static_cast<VulkanEngine*>(glfwGetWindowUserPointer(w))->keyboard_callback(w, key, scancode, action, mods);
		});

}

void VulkanEngine::init_vulkan()
{

	vkboot::VulkanBooter booter(&m_instance,
		&m_debugMessenger,
		&m_gpu,
		&m_device,
		&m_graphicsQueue, &m_surface,
		&m_presentQueue, &m_enableValidationLayers);

	booter.boot_vulkan();

	VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));

	booter.setup_devices();

	create_swapchain();

	create_default_renderpass();

	create_framebuffers();

	init_control_objects();

	create_pipelines();

	m_initialized = true;

}

void VulkanEngine::update()
{
	while (!glfwWindowShouldClose(m_window))
	{
		//I-O
		glfwPollEvents();
		draw();
	}
	VK_CHECK(vkDeviceWaitIdle(m_device));
}

void VulkanEngine::draw()
{
	VK_CHECK(vkWaitForFences(m_device, 1, &m_cmd.inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX));
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device, *m_swapchain.get_swapchain_obj(), UINT64_MAX, m_cmd.imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swap_chain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	VK_CHECK(vkResetFences(m_device, 1, &m_cmd.inFlightFences[m_currentFrame]));
	VK_CHECK(vkResetCommandBuffer(m_cmd.commandBuffers[m_currentFrame], /*VkCommandBufferResetFlagBits*/ 0));

	record_command_buffer(m_cmd.commandBuffers[m_currentFrame], imageIndex);

	//prepare the submission to the queue. 
	//we want to wait on the presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the renderSemaphore, to signal that rendering has finished
	VkSubmitInfo submitInfo = vkinit::submit_info(&m_cmd.commandBuffers[m_currentFrame]);
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { m_cmd.imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	VkSemaphore signalSemaphores[] = { m_cmd.renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_cmd.inFlightFences[m_currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// this will put the image we just rendered to into the visible window.
	// we want to wait on the renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::present_info();
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { *m_swapchain.get_swapchain_obj() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) {
		m_framebufferResized = false;
		recreate_swap_chain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanEngine::cleanup()
{
	if (m_initialized) {
		m_deletionQueue.flush();

		vkDestroyDevice(m_device, nullptr);

		if (m_enableValidationLayers) {
			vkutils::destroy_debug_utils_messenger_EXT(m_instance, m_debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	glfwDestroyWindow(m_window);

	glfwTerminate();
}

void VulkanEngine::create_swapchain()
{
	m_swapchain.create(&m_gpu, &m_device, &m_surface, m_window, &m_windowExtent);

	m_deletionQueue.push_function([=]() {
		//vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		cleanup_swap_chain();

		});
}

void VulkanEngine::create_default_renderpass()
{
	//as in values it sould hace
	//multisampled number
	//stencil
	//depth

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

	if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	m_deletionQueue.push_function([=]() {
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		});

}

void VulkanEngine::create_framebuffers()
{
	auto size = m_swapchain.get_image_views().size();

	m_framebuffers.resize(size);

	VkFramebufferCreateInfo fb_info = vkinit::framebuffer_create_info(m_renderPass, m_windowExtent);
	for (size_t i = 0; i < size; i++) {
		VkImageView attachments[] = {
			m_swapchain.get_image_views()[i]
		};

		fb_info.pAttachments = attachments;


		if (vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

	}


}

void VulkanEngine::init_control_objects()
{
	m_cmd.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
	m_cmd.init(m_device, m_gpu, m_surface);

	m_deletionQueue.push_function([=]() {
		m_cmd.cleanup(m_device);
		});

}




void VulkanEngine::record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo = vkinit::command_buffer_begin_info();
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo = vkinit::renderpass_begin_info(m_renderPass, m_windowExtent, m_framebuffers[imageIndex]);

	//Clear COLOR | DEPTH | STENCIL ??
	VkClearValue clearColor = { {{m_globalParams.clearColor.r, m_globalParams.clearColor.g, m_globalParams.clearColor.b, m_globalParams.clearColor.a}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	//Like bind program
	m_currentPipeline = m_selectedShader == 0 ? &m_pipelines["0"] : &m_pipelines["1"];
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_currentPipeline);

	//Viewport setup
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_windowExtent.width;
	viewport.height = (float)m_windowExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_windowExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}


void VulkanEngine::create_pipelines()
{
	//Populate shader list
	std::vector<Shader> shaders;
	//resources easy route!!!
	shaders.push_back(Shader::read_file("../resources/shaders/test.glsl"));
	shaders.push_back(Shader::read_file("../resources/shaders/red.glsl"));



	//Create standard pipelines info
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
	builder.viewport.width = (float)m_windowExtent.width;
	builder.viewport.height = (float)m_windowExtent.height;
	builder.viewport.minDepth = 0.0f;
	builder.viewport.maxDepth = 1.0f;
	builder.scissor.offset = { 0, 0 };
	builder.scissor.extent = m_windowExtent;

	builder.rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

	builder.multisampling = vkinit::multisampling_state_create_info();

	builder.colorBlendAttachment = vkinit::color_blend_attachment_state();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkinit::pipeline_layout_create_info();


	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	builder.pipelineLayout = m_pipelineLayout;

	//Compile shaders
	int i = 0;
	for each (auto shader in shaders)
	{
		VkShaderModule vertShaderModule{}, fragShaderModule{}, geomShaderModule{}, tessShaderModule{};
		if (shader.vertSource != "") {
			vertShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.vertSource, shader.name + "vert", shaderc_vertex_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule));
		}
		if (shader.fragSource != "") {
			fragShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.fragSource, shader.name + "frag", shaderc_fragment_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule));
		}
		if (shader.geomSource != "") {
			geomShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.geomSource, shader.name + "geom", shaderc_geometry_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_GEOMETRY_BIT, geomShaderModule));
		}
		if (shader.tessSource != "") {
			tessShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(shader.tessSource, shader.name + "tess", shaderc_tess_control_shader, true));
			builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_GEOMETRY_BIT, tessShaderModule));
		}

		m_pipelines[std::to_string(i)] = builder.build_pipeline(m_device, m_renderPass);

		vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
		vkDestroyShaderModule(m_device, geomShaderModule, nullptr);
		vkDestroyShaderModule(m_device, tessShaderModule, nullptr);

		builder.shaderStages.clear();
		i++;
	}


	m_deletionQueue.push_function([=]() {
		for each (auto pipeline in m_pipelines)
		{
			vkDestroyPipeline(m_device, pipeline.second, nullptr);
		}
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		});


}


void VulkanEngine::recreate_swap_chain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	VK_CHECK(vkDeviceWaitIdle(m_device));
	cleanup_swap_chain();
	create_swapchain();
	create_framebuffers();
}

void VulkanEngine::cleanup_swap_chain()
{
	for (size_t i = 0; i < m_framebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
	}

	m_swapchain.cleanup(&m_device);
}




