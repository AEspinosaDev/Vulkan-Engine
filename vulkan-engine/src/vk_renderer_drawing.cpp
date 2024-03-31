/*
	This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

	MIT License

	Copyright (c) 2023 Antonio Espinosa Garcia

	////////////////////////////////////////////////////////////////////////////////////

	In this Renderer's module you will find:

	Implementation of functions focused on managing render passes, drawing data and
	setting up global render states using command.

	////////////////////////////////////////////////////////////////////////////////////
*/

#include <engine/vk_renderer.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void Renderer::set_viewport(VkCommandBuffer &commandBuffer, VkExtent2D extent, float minDepth, float maxDepth, float x, float y, int offsetX, int offsetY)
{
	// Viewport setup
	VkViewport viewport{};
	viewport.x = x;
	viewport.y = y;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = {offsetX, offsetY};
	scissor.extent = extent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::render_forward(VkCommandBuffer &commandBuffer, uint32_t imageIndex, Scene *const scene)
{
	if (!scene->get_lights().empty())
		shadow_pass(m_frames[m_currentFrame].commandBuffer, scene);

	forward_pass(m_frames[m_currentFrame].commandBuffer, imageIndex, scene);
}

void Renderer::render_deferred(VkCommandBuffer &commandBuffer, uint32_t imageIndex, Scene *const scene)
{
	if (!scene->get_lights().empty())
		shadow_pass(m_frames[m_currentFrame].commandBuffer, scene);

	// geometry_pass();

	// lighting_pass();
}

void Renderer::forward_pass(VkCommandBuffer &commandBuffer, uint32_t imageIndex, Scene *const scene)
{

	VkClearValue clearColor = {{{m_settings.clearColor.r, m_settings.clearColor.g, m_settings.clearColor.b, m_settings.clearColor.a}}};
	VkClearValue clearDepth;
	clearDepth.depthStencil.depth = 1.f;
	
	RenderPass::begin(commandBuffer,m_renderPasses[DEFAULT],*m_window->get_extent(),{clearColor, clearColor,clearDepth},imageIndex);
	
	set_viewport(commandBuffer, *m_window->get_extent());

	vkCmdSetDepthTestEnable(commandBuffer, m_settings.depthTest);

	vkCmdSetDepthWriteEnable(commandBuffer, m_settings.depthWrite);

	// vkCmdSetRasterizationSamplesEXT(commandBuffer,VK_SAMPLE_COUNT_8_BIT);

	if (scene->get_active_camera() && scene->get_active_camera()->is_active())
	{

		unsigned int mesh_idx = 0;
		for (Mesh *m : scene->get_meshes())
		{
			if (m)
			{
				if (m->is_active() &&																	  // Check if is active
					m->get_num_geometries() > 0 &&														  // Check if has geometry
					m->get_bounding_volume()->is_on_frustrum(scene->get_active_camera()->get_frustrum())) // Check if is inside frustrum
				{
					// Offset calculation
					uint32_t objectOffset = m_frames[m_currentFrame].objectUniformBuffer.strideSize * mesh_idx;
					uint32_t globalOffset = m_globalUniformsBuffer.strideSize * m_currentFrame;

					for (size_t i = 0; i < m->get_num_geometries(); i++)
					{
						Geometry *g = m->get_geometry(i);

						Material *mat = m->get_material(g->m_materialID);

						// Setup per object render state
						if (m_settings.depthTest)
							vkCmdSetDepthTestEnable(commandBuffer, mat->get_parameters().depthTest);
						if (m_settings.depthWrite)
							vkCmdSetDepthWriteEnable(commandBuffer, mat->get_parameters().depthWrite);
						vkCmdSetCullMode(commandBuffer, mat->get_parameters().faceCulling ? (VkCullModeFlags)mat->get_parameters().culling : VK_CULL_MODE_NONE);

						// Bind pipeline
						vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->m_shaderPass->pipeline);

						// GLOBAL LAYOUT BINDING
						uint32_t globalOffsets[] = {globalOffset, globalOffset};
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->m_shaderPass->pipelineLayout, 0, 1, &m_globalDescriptor.descriptorSet, 2, globalOffsets);

						// PER OBJECT LAYOUT BINDING
						uint32_t objectOffsets[] = {objectOffset, objectOffset};
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->m_shaderPass->pipelineLayout, 1, 1, &m_frames[m_currentFrame].objectDescriptor.descriptorSet, 2, objectOffsets);

						// TEXTURE LAYOUT BINDING
						if (mat->m_shaderPass->settings.descriptorSetLayoutIDs[DescriptorLayoutType::TEXTURE_LAYOUT])
							vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->m_shaderPass->pipelineLayout, 2, 1, &mat->m_textureDescriptor.descriptorSet, 0, nullptr);

						draw_geometry(commandBuffer, g);
					}
				}
			}
			mesh_idx++;
		}
	}

	if (m_settings.enableUI && m_gui)
		m_gui->upload_draw_data(commandBuffer);

	RenderPass::end(commandBuffer);
}

void Renderer::shadow_pass(VkCommandBuffer &commandBuffer, Scene *const scene)
{
	const uint32_t SHADOW_RES = (uint32_t)m_settings.shadowResolution;
	VkClearValue clearDepth;
	clearDepth.depthStencil = {1.0f, 0};

	RenderPass::begin(commandBuffer,m_renderPasses[SHADOW], {SHADOW_RES, SHADOW_RES}, {clearDepth});

	set_viewport(commandBuffer, {SHADOW_RES, SHADOW_RES});

	// float depthBiasConstant = 1.25f;
	vkCmdSetDepthBiasEnable(commandBuffer, scene->get_lights()[0]->get_use_vulkan_bias());
	float depthBiasConstant = scene->get_lights()[0]->get_shadow_bias();
	float depthBiasSlope = 0.0f;
	vkCmdSetDepthBias(
		commandBuffer,
		depthBiasConstant,
		0.0f,
		depthBiasSlope);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shaderPasses["shadows"]->pipeline);

	int mesh_idx = 0;
	for (Mesh *m : scene->get_meshes())
	{
		if (m)
		{
			if (m->is_active() && m->get_cast_shadows() && m->get_num_geometries() > 0)
			{
				uint32_t objectOffset = m_frames[m_currentFrame].objectUniformBuffer.strideSize * mesh_idx;
				uint32_t globalOffset = m_globalUniformsBuffer.strideSize * m_currentFrame;

				for (size_t i = 0; i < m->get_num_geometries(); i++)
				{
					// GLOBAL LAYOUT BINDING
					uint32_t globalOffsets[] = {globalOffset, globalOffset};
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shaderPasses["shadows"]->pipelineLayout, 0, 1, &m_globalDescriptor.descriptorSet, 2, globalOffsets);
					// PER OBJECT LAYOUT BINDING
					uint32_t objectOffsets[] = {objectOffset, objectOffset};
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shaderPasses["shadows"]->pipelineLayout, 1, 1,
											&m_frames[m_currentFrame].objectDescriptor.descriptorSet, 2, objectOffsets);

					draw_geometry(commandBuffer, m->get_geometry(i));
				}
			}
			mesh_idx++;
		}
	}

	RenderPass::end(commandBuffer);
}
void Renderer::geometry_pass(VkCommandBuffer &commandBuffer, Scene *const scene)
{
}
void Renderer::lighting_pass(VkCommandBuffer &commandBuffer, Scene *const scene)
{
}
void Renderer::draw_geometry(VkCommandBuffer &commandBuffer, Geometry *const g)
{

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

VULKAN_ENGINE_NAMESPACE_END