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
#include <engine/vk_renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::init_renderpasses()
{

	RenderPassBuilder builder;

	// ---------- DEFAULT RENDER PASS ------------

	VkSampleCountFlagBits samples = (VkSampleCountFlagBits)m_settings.AAtype;
	bool multisampled = samples > VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentDescription colorAttachment = init::attachment_description(static_cast<VkFormat>(m_settings.colorFormat),
																		   multisampled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_UNDEFINED, samples);
	builder.add_attachment({colorAttachment, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT});

	if (multisampled)
	{
		VkAttachmentDescription resolveAttachment = init::attachment_description(static_cast<VkFormat>(m_settings.colorFormat), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		builder.add_attachment({resolveAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT});
	}

	VkAttachmentDescription depthAttachment = init::attachment_description(static_cast<VkFormat>(m_settings.depthFormat),
																		   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, samples);
	builder.add_attachment({depthAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT});

	VkSubpassDependency colorDep = init::subpass_dependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
															0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	builder.add_dependency(colorDep);
	VkSubpassDependency depthDep = init::subpass_dependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
															0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
	builder.add_dependency(depthDep);

	VkAttachmentReference colorRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkAttachmentReference depthRef = init::attachment_reference(static_cast<uint32_t>(builder.attachments.size()) - 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	VkSubpassDescription defaultSubpass = init::subpass_description(1, &colorRef, depthRef);
	if (multisampled)
	{
		VkAttachmentReference resolveRef = init::attachment_reference(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		defaultSubpass.pResolveAttachments = &resolveRef;
	}
	builder.add_subpass(defaultSubpass);

	m_renderPasses[DEFAULT] = builder.build_renderpass(m_device);

	m_deletionQueue.push_function([=]()
								  { m_renderPasses[DEFAULT].cleanup(m_device); });

	create_framebuffer(m_renderPasses[DEFAULT], *m_window->get_extent(), 1, static_cast<uint32_t>(m_settings.bufferingType) + 1);

	builder.clear_cache();

	// ---------- SHADOW RENDER PASS -----------

	depthAttachment = init::attachment_description(static_cast<VkFormat>(m_settings.depthFormat),
												   VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT, false);
	builder.add_attachment({depthAttachment,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							VK_IMAGE_ASPECT_DEPTH_BIT,
							VK_IMAGE_VIEW_TYPE_2D_ARRAY});

	VkSubpassDependency earlyDepthDep = init::subpass_dependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
																 VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
	earlyDepthDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	builder.add_dependency(earlyDepthDep);

	VkSubpassDependency lateDepthDep = init::subpass_dependency(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
																VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, 0, VK_SUBPASS_EXTERNAL);
	lateDepthDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	builder.add_dependency(lateDepthDep);

	depthRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	VkSubpassDescription depthSubpass = init::subpass_description(0, VK_NULL_HANDLE, depthRef);
	builder.add_subpass(depthSubpass);

	m_renderPasses[SHADOW] = builder.build_renderpass(m_device);

	m_deletionQueue.push_function([=]()
								  { m_renderPasses[SHADOW].cleanup(m_device); });

	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;

	create_framebuffer(m_renderPasses[SHADOW], {SHADOW_RES, SHADOW_RES}, VK_MAX_LIGHTS);
	m_renderPasses[SHADOW].isFramebufferRecreatable = false;

	builder.clear_cache();

	/// ------ Geometry pass ------

	/// -------- Light pass --------
}

void Renderer::create_framebuffer(RenderPass &pass, VkExtent2D extent, uint32_t layers, uint32_t count)
{
	pass.extent = extent;
	pass.framebuffers.resize(count);

	std::vector<VkImageView> imgAttachments;
	imgAttachments.resize(pass.attachmentsInfo.size());

	if (pass.textureAttachments.empty())
		pass.textureAttachments.resize(pass.attachmentsInfo.size(), nullptr);

	bool isDefaultRenderPass{false};
	size_t presentViewIndex{0};

	for (size_t i = 0; i < pass.attachmentsInfo.size(); i++)
	{
		// Create image and image view for framebuffer
		if (pass.attachmentsInfo[i].description.finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			Image image;
			image.init(m_memory, pass.attachmentsInfo[i].description.format,
					   pass.attachmentsInfo[i].viewUsage, {extent.width, extent.height, 1}, false, pass.attachmentsInfo[i].description.samples, layers);
			image.create_view(m_device, pass.attachmentsInfo[i].viewAspect, pass.attachmentsInfo[i].viewType);
			imgAttachments[i] = image.view;

			// Save it in the texture inside the Renderpass
			if (!pass.textureAttachments[i])
				pass.textureAttachments[i] = new Texture();
			pass.textureAttachments[i]->m_image = image;
		}
		else
		{
			isDefaultRenderPass = true;
			presentViewIndex = i;
		}
	}

	for (size_t fb = 0; fb < count; fb++)
	{
		if (isDefaultRenderPass)
			imgAttachments[presentViewIndex] = m_swapchain.get_present_images()[fb].view;

		VkFramebufferCreateInfo fbInfo = init::framebuffer_create_info(pass.obj, pass.extent);
		fbInfo.pAttachments = imgAttachments.data();
		fbInfo.attachmentCount = (uint32_t)imgAttachments.size();
		fbInfo.layers = layers;

		if (vkCreateFramebuffer(m_device, &fbInfo, nullptr, &pass.framebuffers[fb]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
void Renderer::clean_framebuffer(RenderPass &pass)
{

	for (VkFramebuffer &fb : pass.framebuffers)
		vkDestroyFramebuffer(m_device, fb, nullptr);

	for (size_t i = 0; i < pass.textureAttachments.size(); i++)
	{
		if (pass.textureAttachments[i])
			pass.textureAttachments[i]->m_image.cleanup(m_device, m_memory);
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

	VkFenceCreateInfo uploadFenceCreateInfo = init::fence_create_info();

	VK_CHECK(vkCreateFence(m_device, &uploadFenceCreateInfo, nullptr, &m_uploadContext.uploadFence));
	m_deletionQueue.push_function([=]()
								  { vkDestroyFence(m_device, m_uploadContext.uploadFence, nullptr); });

	VkCommandPoolCreateInfo uploadCommandPoolInfo = init::command_pool_create_info(boot::find_queue_families(m_gpu, *m_window->get_surface()).graphicsFamily.value());
	VK_CHECK(vkCreateCommandPool(m_device, &uploadCommandPoolInfo, nullptr, &m_uploadContext.commandPool));

	m_deletionQueue.push_function([=]()
								  { vkDestroyCommandPool(m_device, m_uploadContext.commandPool, nullptr); });

	// allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = init::command_buffer_allocate_info(m_uploadContext.commandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_uploadContext.commandBuffer));
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
	VkDescriptorSetLayoutBinding textureBindings[] = {textureBinding0, textureBinding1, textureBinding2, textureBinding3, textureBinding4, textureBinding5};
	m_descriptorMng.set_layout(DescriptorLayoutType::TEXTURE_LAYOUT, textureBindings, 6);

	const size_t strideSize = (utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu) + utils::pad_uniform_buffer_size(sizeof(SceneUniforms), m_gpu));
	const size_t globalUBOSize = MAX_FRAMES_IN_FLIGHT * strideSize;
	m_globalUniformsBuffer.init(m_memory, globalUBOSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)strideSize);

	m_deletionQueue.push_function([=]()
								  { m_globalUniformsBuffer.cleanup(m_memory); });

	m_descriptorMng.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_globalDescriptor);

	m_descriptorMng.set_descriptor_write(&m_globalUniformsBuffer, sizeof(CameraUniforms), 0,
										 &m_globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);

	m_descriptorMng.set_descriptor_write(&m_globalUniformsBuffer, sizeof(SceneUniforms), utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_gpu),
										 &m_globalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{

		const size_t strideSize = (utils::pad_uniform_buffer_size(sizeof(ObjectUniforms), m_gpu) + utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_gpu));
		m_frames[i].objectUniformBuffer.init(m_memory, VK_MAX_OBJECTS * strideSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)strideSize);
		m_deletionQueue.push_function([=]()
									  { m_frames[i].objectUniformBuffer.cleanup(m_memory); });

		m_descriptorMng.allocate_descriptor_set(DescriptorLayoutType::OBJECT_LAYOUT, &m_frames[i].objectDescriptor);

		m_descriptorMng.set_descriptor_write(&m_frames[i].objectUniformBuffer, sizeof(ObjectUniforms), 0, &m_frames[i].objectDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0);

		m_descriptorMng.set_descriptor_write(&m_frames[i].objectUniformBuffer, sizeof(MaterialUniforms), utils::pad_uniform_buffer_size(sizeof(MaterialUniforms), m_gpu), &m_frames[i].objectDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1);
	}

	m_deletionQueue.push_function([=]()
								  { m_descriptorMng.cleanup(); });
}

void Renderer::init_shaderpasses()
{
	PipelineBuilder builder;

	// Default geometry assembly values
	builder.vertexInputInfo = init::vertex_input_state_create_info();
	builder.inputAssembly = init::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	auto bindingDescription = Vertex::getBindingDescription();
	builder.vertexInputInfo.vertexBindingDescriptionCount = 1;
	builder.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	// Viewport
	builder.viewport.x = 0.0f;
	builder.viewport.y = 0.0f;
	builder.viewport.width = (float)m_window->get_extent()->width;
	builder.viewport.height = (float)m_window->get_extent()->height;
	builder.viewport.minDepth = 0.0f;
	builder.viewport.maxDepth = 1.0f;
	builder.scissor.offset = {0, 0};
	builder.scissor.extent = *m_window->get_extent();

	builder.rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

	builder.depthStencil = init::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS);

	builder.multisampling = init::multisampling_state_create_info((VkSampleCountFlagBits)m_settings.AAtype);

	builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
	builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
	builder.dynamicStates.push_back(VK_DYNAMIC_STATE_CULL_MODE);
	// builder.dynamicStates.push_back(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);

	// Setup shaderpasses
	std::string shaderDir(VK_SHADER_DIR);
	m_shaderPasses["unlit"] = new ShaderPass(shaderDir + "unlit.glsl");
	m_shaderPasses["unlit"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
																{DescriptorLayoutType::OBJECT_LAYOUT, true},
																{DescriptorLayoutType::TEXTURE_LAYOUT, false}};
	m_shaderPasses["unlit"]->settings.attributes = {{VertexAttributeType::POSITION, true},
													{VertexAttributeType::NORMAL, false},
													{VertexAttributeType::UV, false},
													{VertexAttributeType::TANGENT, false},
													{VertexAttributeType::COLOR, false}};

	m_shaderPasses["phong"] = new ShaderPass(shaderDir + "phong.glsl");
	m_shaderPasses["phong"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
																{DescriptorLayoutType::OBJECT_LAYOUT, true},
																{DescriptorLayoutType::TEXTURE_LAYOUT, true}};
	m_shaderPasses["phong"]->settings.attributes = {{VertexAttributeType::POSITION, true},
													{VertexAttributeType::NORMAL, true},
													{VertexAttributeType::UV, true},
													{VertexAttributeType::TANGENT, false},
													{VertexAttributeType::COLOR, false}};

	m_shaderPasses["physical"] = new ShaderPass(shaderDir + "physically_based.glsl");
	m_shaderPasses["physical"]->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
																   {DescriptorLayoutType::OBJECT_LAYOUT, true},
																   {DescriptorLayoutType::TEXTURE_LAYOUT, true}};
	m_shaderPasses["physical"]->settings.attributes = {{VertexAttributeType::POSITION, true},
													   {VertexAttributeType::NORMAL, true},
													   {VertexAttributeType::UV, true},
													   {VertexAttributeType::TANGENT, true},
													   {VertexAttributeType::COLOR, false}};

	for (auto pair : m_shaderPasses)
	{
		ShaderPass *pass = pair.second;

		ShaderPass::build_shader_stages(m_device, *pass);

		builder.build_pipeline_layout(m_device, m_descriptorMng, *pass);
		builder.build_pipeline(m_device, m_renderPasses[DEFAULT].obj, *pass);

		m_deletionQueue.push_function([=]()
									  { pass->cleanup(m_device); });
	}

	if (m_initialized)
		return;
	// DEPTH PASS
	ShaderPass *depthPass = new ShaderPass(shaderDir + "shadows.glsl");
	depthPass->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
												  {DescriptorLayoutType::OBJECT_LAYOUT, true},
												  {DescriptorLayoutType::TEXTURE_LAYOUT, false}};
	depthPass->settings.attributes = {{VertexAttributeType::POSITION, true},
									  {VertexAttributeType::NORMAL, false},
									  {VertexAttributeType::UV, false},
									  {VertexAttributeType::TANGENT, false},
									  {VertexAttributeType::COLOR, false}};

	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
	VkExtent2D shadowExtent{SHADOW_RES, SHADOW_RES};
	builder.viewport.width = (float)shadowExtent.width;
	builder.viewport.height = (float)shadowExtent.height;
	builder.scissor.extent = shadowExtent;
	builder.rasterizer.depthBiasEnable = VK_TRUE;

	builder.dynamicStates.pop_back();
	builder.dynamicStates.pop_back();
	builder.dynamicStates.pop_back();
	// builder.dynamicStates.pop_back();
	builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	builder.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
	builder.colorBlending.attachmentCount = 0;
	//  builder.dynamicState = VK_TRUE;

	builder.multisampling = init::multisampling_state_create_info(VK_SAMPLE_COUNT_1_BIT);

	ShaderPass::build_shader_stages(m_device, *depthPass);

	builder.build_pipeline_layout(m_device, m_descriptorMng, *depthPass);
	builder.build_pipeline(m_device, m_renderPasses[SHADOW].obj, *depthPass);

	m_shaderPasses["shadows"] = depthPass;

	m_deletionQueue.push_function([=]()
								  { depthPass->cleanup(m_device); });
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

	m_swapchain.cleanup(m_device, m_memory);
	m_swapchain.create(m_gpu, m_device, *m_window->get_surface(), m_window->get_window_obj(), *m_window->get_extent(), static_cast<uint32_t>(m_settings.bufferingType),
					   static_cast<VkFormat>(m_settings.colorFormat), static_cast<VkPresentModeKHR>(m_settings.screenSync));

	for (auto &pass : m_renderPasses)
	{
		if (pass.second.isFramebufferRecreatable)
			clean_framebuffer(pass.second);
	}
	create_framebuffer(m_renderPasses[DEFAULT], *m_window->get_extent(), 1, static_cast<uint32_t>(m_settings.bufferingType) + 1);
}

void Renderer::immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function)
{
	VkCommandBuffer cmd = m_uploadContext.commandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = init::submit_info(&cmd);

	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_uploadContext.uploadFence));

	vkWaitForFences(m_device, 1, &m_uploadContext.uploadFence, true, 9999999999);
	vkResetFences(m_device, 1, &m_uploadContext.uploadFence);

	vkResetCommandPool(m_device, m_uploadContext.commandPool, 0);
}

VULKAN_ENGINE_NAMESPACE_END