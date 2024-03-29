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

void Renderer::create_swapchain()
{
	m_swapchain.create(m_gpu, m_device, *m_window->get_surface(), m_window->get_window_obj(), *m_window->get_extent(), (VkFormat)m_settings.colorFormat, (VkPresentModeKHR)m_settings.screenSync);

	// COLOR BUFFER SETUP
	m_swapchain.create_colorbuffer(m_device, m_memory, *m_window->get_extent(), (VkSampleCountFlagBits)m_settings.AAtype);
	// DEPTH STENCIL BUFFER SETUP
	if (m_settings.depthTest)
		m_swapchain.create_depthbuffer(m_device, m_memory, *m_window->get_extent(), (VkSampleCountFlagBits)m_settings.AAtype);
}

void Renderer::init_renderpasses()
{

	RenderPassBuilder builder;

	// ---------- DEFAULT RENDER PASS ------------

	VkSampleCountFlagBits samples = (VkSampleCountFlagBits)m_settings.AAtype;
	bool multisampled = samples > VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentDescription colorAttachment = init::attachment_description(m_swapchain.get_image_format(),
																		   multisampled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_UNDEFINED, samples);
	builder.add_attachment({colorAttachment});
	VkAttachmentDescription depthAttachment = init::attachment_description(m_swapchain.get_depthbuffer().format,
																		   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, samples);
	builder.add_attachment({depthAttachment});

	if (multisampled)
	{
		VkAttachmentDescription resolveAttachment = init::attachment_description(m_swapchain.get_image_format(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		builder.add_attachment({resolveAttachment});
	}

	VkSubpassDependency colorDep = init::subpass_dependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
															0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	builder.add_dependency(colorDep);
	VkSubpassDependency depthDep = init::subpass_dependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
															0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
	builder.add_dependency(depthDep);

	VkAttachmentReference colorRef = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkAttachmentReference depthRef = init::attachment_reference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	VkSubpassDescription defaultSubpass = init::subpass_description(1, &colorRef, depthRef);
	if (multisampled)
	{
		VkAttachmentReference resolveRef = init::attachment_reference(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		defaultSubpass.pResolveAttachments = &resolveRef;
	}
	builder.add_subpass(defaultSubpass);

	m_renderPasses[DEFAULT] = builder.build_renderpass(m_device);

	m_deletionQueue.push_function([=]()
								  { vkDestroyRenderPass(m_device, m_renderPasses[DEFAULT].obj, nullptr); });

	// if (m_initialized)
	// 	return;

	builder.dependencies.clear();
	builder.subpasses.clear();
	builder.attachments.clear();

	// ---------- SHADOW RENDER PASS -----------

	depthAttachment = init::attachment_description(m_swapchain.get_depthbuffer().format,
												   VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_SAMPLE_COUNT_1_BIT, false);
	builder.add_attachment({depthAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY});

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
								  { vkDestroyRenderPass(m_device, m_renderPasses[SHADOW].obj, nullptr); });

	builder.dependencies.clear();
	builder.subpasses.clear();
	builder.attachments.clear();

	/// ------ Geometry pass ------

	/// -------- Light pass --------
}

void Renderer::init_framebuffers()
{
	// FOR THE SWAPCHAIN
	m_swapchain.create_framebuffers(m_device, m_renderPasses[DEFAULT].obj, *m_window->get_extent(), (VkSampleCountFlagBits)m_settings.AAtype);

	// FOR SHADOW PASS
	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;

	Image shadowImage;
	shadowImage.init(m_memory, m_swapchain.get_depthbuffer().format,
					 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, {SHADOW_RES, SHADOW_RES, 1}, false, VK_SAMPLE_COUNT_1_BIT, VK_MAX_LIGHTS);

	shadowImage.create_view(m_device, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

	VkExtent2D shadowRes{SHADOW_RES, SHADOW_RES};
	VkFramebufferCreateInfo shadow_fb_info = init::framebuffer_create_info(m_renderPasses[SHADOW].obj, shadowRes);
	shadow_fb_info.pAttachments = &shadowImage.view;
	shadow_fb_info.layers = VK_MAX_LIGHTS;

	if (vkCreateFramebuffer(m_device, &shadow_fb_info, nullptr, &m_renderPasses[SHADOW].framebuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shadow framebuffer!");
	}

	Texture *shadowsTexture = new Texture();
	// m_shadowsTexture->m_image = m_renderPasses[SHADOW].textureAttachments[0];
	shadowsTexture->m_image = shadowImage;

	VkSamplerCreateInfo sampler = init::sampler_create_info(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, 1.0f, false, 1.0f, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	sampler.maxAnisotropy = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VK_CHECK(vkCreateSampler(m_device, &sampler, nullptr, &shadowsTexture->m_sampler));

	m_renderPasses[SHADOW].textureAttachments.push_back(shadowsTexture);

	m_deletionQueue.push_function([=]()
								  { shadowsTexture->cleanup(m_device, m_memory); });

	m_deletionQueue.push_function([=]()
								  { vkDestroyFramebuffer(m_device, m_renderPasses[SHADOW].framebuffer, nullptr); });
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

	// m_descriptorMng.allocate_descriptor_set(2, &m_textureDescriptor);
	// m_descriptorMng.set_descriptor_write(m_shadowTexture->m_sampler, m_shadowTexture->m_image.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &m_textureDescriptor, 0);

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
	create_swapchain();
	m_swapchain.create_framebuffers(m_device, m_renderPasses[DEFAULT].obj, *m_window->get_extent(), (VkSampleCountFlagBits)m_settings.AAtype);
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