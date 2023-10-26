#include "vk_engine.h"





void VulkanEngine::init_window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_window = glfwCreateWindow(m_globalParams.width, m_globalParams.height, "Vulkan Renderer", nullptr, nullptr);

	//WINDOW CALLBACKS
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	glfwSetKeyCallback(m_window, onKeyPressedCallback);

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

	init_commands();

	create_sync_objects();

	create_pipelines();

	m_globalParams.initialized = true;

}

void VulkanEngine::update()
{
	while (!glfwWindowShouldClose(m_window))
	{
		//I-O
		glfwPollEvents();
		draw();
	}
	vkDeviceWaitIdle(m_device);
}

void VulkanEngine::draw()
{
	vkWaitForFences(m_device, 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device, *m_swapchain.get_swapchain_obj(), UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreate_swap_chain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(m_device, 1, &m_inFlightFences[currentFrame]);

	vkResetCommandBuffer(m_commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	record_command_buffer(m_commandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { *m_swapchain.get_swapchain_obj() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreate_swap_chain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanEngine::cleanup()
{
	cleanup_swap_chain();

	{
		vkDestroyPipeline(m_device, m_currentPipeline, nullptr);
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	}

	vkDestroyDevice(m_device, nullptr);

	if (m_enableValidationLayers) {
		vkutils::destroy_debug_utils_messenger_EXT(m_instance, m_debugMessenger, nullptr);
	}
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);

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

		/*m_deletionQueue.push_function([=]() {
			vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
			vkDestroyImageView(m_device, _swapchainImageViews[i], nullptr);
			});*/

	}


}

void VulkanEngine::init_commands()
{
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(vkboot::find_queue_families(m_gpu, m_surface).graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool));

	//allocate the default command buffer that we will use for rendering
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(m_commandPool, (uint32_t)m_commandBuffers.size());

	VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, m_commandBuffers.data()));

	m_deletionQueue.push_function([=]() {
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		});
}








void VulkanEngine::record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_windowExtent;

	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipeline);

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

void VulkanEngine::create_sync_objects()
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

		VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_inFlightFences[i]));
		m_deletionQueue.push_function([=]() {
			vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
			});
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i]));
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i]));
		m_deletionQueue.push_function([=]() {
			vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
			});
	}


}

void VulkanEngine::create_pipelines()
{
	
	//for all pipelines or shaders


	Shader testShader = Shader::read_file("../resources/shaders/test.glsl");
	VkShaderModule vertShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(testShader.vertSource, "test vert", shaderc_vertex_shader, true));
	VkShaderModule fragShaderModule = Shader::create_shader_module(m_device, Shader::compile_shader(testShader.fragSource, "test frag", shaderc_fragment_shader, true));
	
	PipelineBuilder builder;

	builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertShaderModule));
	builder.shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderModule));

	builder.vertexInputInfo = vkinit::vertex_input_state_create_info();

	builder.inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

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

	m_currentPipeline = builder.build_pipeline(m_device, m_renderPass);

	vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_device, vertShaderModule, nullptr);

	builder.shaderStages.clear();

	m_deletionQueue.push_function([=]() {
		vkDestroyPipeline(m_device, m_currentPipeline, nullptr);
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




