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

	// DEFAULT RENDER PASS

	builder.add_color_attachment(m_swapchain.get_image_format(), (VkSampleCountFlagBits)m_settings.AAtype);

	builder.setup_depth_attachment(m_swapchain.get_depthbuffer().format, (VkSampleCountFlagBits)m_settings.AAtype);

	m_renderPasses[DEFAULT] = builder.build_renderpass(m_device, true, true);

	m_deletionQueue.push_function([=]()
								  { vkDestroyRenderPass(m_device, m_renderPasses[DEFAULT], nullptr); });

	if (m_initialized)
		return;

	// SHADOW RENDER PASS
	builder.setup_depth_attachment(m_swapchain.get_depthbuffer().format, VK_SAMPLE_COUNT_1_BIT, false, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

	std::vector<VkSubpassDependency> dependencies;
	dependencies.resize(2);

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	m_renderPasses[SHADOW] = builder.build_renderpass(m_device, false, true, dependencies);

	m_deletionQueue.push_function([=]()
								  { vkDestroyRenderPass(m_device, m_renderPasses[SHADOW], nullptr); });
}

void Renderer::init_framebuffers()
{
	// FOR THE SWAPCHAIN
	m_swapchain.create_framebuffers(m_device, m_renderPasses[DEFAULT], *m_window->get_extent(), (VkSampleCountFlagBits)m_settings.AAtype);

	// FOR SHADOW PASS
	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;

	m_shadowsTexture = new Texture();
	Image shadowImage;
	shadowImage.init(m_memory, m_swapchain.get_depthbuffer().format,
					 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, {SHADOW_RES, SHADOW_RES, 1}, false, VK_SAMPLE_COUNT_1_BIT, VK_MAX_LIGHTS);

	shadowImage.create_view(m_device, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
	m_shadowsTexture->m_image = shadowImage;

	VkSamplerCreateInfo sampler = init::sampler_create_info(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, 1.0f, false, 1.0f, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	sampler.maxAnisotropy = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VK_CHECK(vkCreateSampler(m_device, &sampler, nullptr, &m_shadowsTexture->m_sampler));

	m_deletionQueue.push_function([=]()
								  { m_shadowsTexture->cleanup(m_device, m_memory); });

	//	To light
	VkExtent2D shadowRes{SHADOW_RES, SHADOW_RES};
	VkFramebufferCreateInfo shadow_fb_info = init::framebuffer_create_info(m_renderPasses[SHADOW], shadowRes);
	shadow_fb_info.pAttachments = &m_shadowsTexture->m_image.view;
	shadow_fb_info.layers = VK_MAX_LIGHTS;

	if (vkCreateFramebuffer(m_device, &shadow_fb_info, nullptr, &m_shadowFramebuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shadow framebuffer!");
	}

	m_deletionQueue.push_function([=]()
								  { vkDestroyFramebuffer(m_device, m_shadowFramebuffer, nullptr); });
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
		builder.build_pipeline(m_device, m_renderPasses[DEFAULT], *pass);

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
	builder.build_pipeline(m_device, m_renderPasses[SHADOW], *depthPass);

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
	m_swapchain.create_framebuffers(m_device, m_renderPasses[DEFAULT], *m_window->get_extent(), (VkSampleCountFlagBits)m_settings.AAtype);
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