// #include <engine/core/passes/subsurface_pass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN
// using namespace Graphics;
// namespace Core {

// void SubSurfacePass::resize_attachments() {
//     BaseGraphicPass::resize_attachments();

//     // Update descriptor of previous framebuffer
//     m_descriptorPool.update_descriptor(&m_interAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptor, 0);
//     m_descriptorPool.update_descriptor(&m_interAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptor, 1);
// }
// void SubSurfacePass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

//     // Init and configure local descriptors
//     m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 2);

//     LayoutBinding LUTBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
//     LayoutBinding SKYBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
//     m_descriptorPool.set_layout(GLOBAL_LAYOUT, {LUTBinding, SKYBinding});

//     m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_imageDescriptor);

//     m_descriptorPool.update_descriptor(&m_interAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptor, 0);
//     m_descriptorPool.update_descriptor(&m_interAttachments[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_imageDescriptor, 1);
// }
// void SubSurfacePass::setup_shader_passes() {

//     GraphicShaderPass* ttPass = new GraphicShaderPass(
//         m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/env/sky_tt_compute.glsl");
//     ttPass->settings.pushConstants = {
//         PushConstant(SHADER_STAGE_FRAGMENT, sizeof(Core::SkySettings) + sizeof(AerosolParams))};
//     ttPass->settings.descriptorSetLayoutIDs = {{0, true}};
//     ttPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
//                                                {NORMAL_ATTRIBUTE, false},
//                                                {UV_ATTRIBUTE, true},
//                                                {TANGENT_ATTRIBUTE, false},
//                                                {COLOR_ATTRIBUTE, false}};

//     ttPass->build_shader_stages();
//     ttPass->build(m_descriptorPool);

//     m_shaderPasses["tt"] = ttPass;

//     GraphicShaderPass* skyPass = new GraphicShaderPass(
//         m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/env/sky_generation.glsl");
//     skyPass->settings.pushConstants = {
//         PushConstant(SHADER_STAGE_FRAGMENT, sizeof(Core::SkySettings) + sizeof(AerosolParams))};
//     skyPass->settings.descriptorSetLayoutIDs = ttPass->settings.descriptorSetLayoutIDs;
//     skyPass->graphicSettings.attributes      = ttPass->graphicSettings.attributes;

//     skyPass->build_shader_stages();
//     skyPass->build(m_descriptorPool);

//     m_shaderPasses["sky"] = skyPass;

//     GraphicShaderPass* projPass = new GraphicShaderPass(
//         m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/env/sky_projection.glsl");
//     projPass->settings.pushConstants          = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(int))};
//     projPass->settings.descriptorSetLayoutIDs = ttPass->settings.descriptorSetLayoutIDs;
//     projPass->graphicSettings.attributes      = ttPass->graphicSettings.attributes;

//     projPass->build_shader_stages();
//     projPass->build(m_descriptorPool);

//     m_shaderPasses["proj"] = projPass;
// }

// void SubSurfacePass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
//     PROFILING_EVENT()
//     if (!scene->get_skybox())
//         return;

//     CommandBuffer cmd = currentFrame.commandBuffer;

//     struct PassThroughSettings {
//         SkySettings   sky;
//         AerosolParams aerosol;
//     };

//     PassThroughSettings passSettings;
//     passSettings.sky     = scene->get_skybox()->get_sky_settings();
//     passSettings.aerosol = get_aerosol_params(passSettings.sky.aerosol);

//     /* Transmittance LUT Generation*/
//     // ------------------------------------
//     cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
//     cmd.set_viewport(m_imageExtent);
//     ShaderPass* shaderPass = m_shaderPasses["tt"];
//     cmd.bind_shaderpass(*shaderPass);
//     cmd.push_constants(
//         *shaderPass, SHADER_STAGE_FRAGMENT, &passSettings, sizeof(Core::SkySettings) + sizeof(AerosolParams));
//     cmd.draw_geometry(*get_VAO(BasePass::vignette));
//     cmd.end_renderpass(m_renderpass, m_framebuffers[0]);

//     /* Sky Generation*/
//     // ------------------------------------
//     cmd.begin_renderpass(m_renderpass, m_framebuffers[1]);
//     cmd.set_viewport(m_imageExtent);
//     shaderPass = m_shaderPasses["sky"];
//     cmd.bind_shaderpass(*shaderPass);
//     cmd.push_constants(
//         *shaderPass, SHADER_STAGE_FRAGMENT, &passSettings, sizeof(Core::SkySettings) + sizeof(AerosolParams));
//     cmd.bind_descriptor_set(m_imageDescriptor, 0, *shaderPass);
//     cmd.draw_geometry(*get_VAO(BasePass::vignette));
//     cmd.end_renderpass(m_renderpass, m_framebuffers[1]);

//     /* Sky Generation*/
//     // ------------------------------------
//     cmd.begin_renderpass(m_renderpass, m_framebuffers[2]);
//     cmd.set_viewport(m_imageExtent);
//     shaderPass = m_shaderPasses["proj"];
//     cmd.bind_shaderpass(*shaderPass);
//     int projectionType = passSettings.sky.useForIBL;
//     cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &projectionType, sizeof(int));
//     cmd.bind_descriptor_set(m_imageDescriptor, 0, *shaderPass);
//     cmd.draw_geometry(*get_VAO(BasePass::vignette));
//     cmd.end_renderpass(m_renderpass, m_framebuffers[2]);

//     /* Sky is updated, set to sleep */
//     if (passSettings.sky.updateType == UpdateType::ON_DEMAND)
//         set_active(false);
// }
// } // namespace Core

// VULKAN_ENGINE_NAMESPACE_END