// #include <engine/graphics/renderpasses/ssao_pass.h>

// VULKAN_ENGINE_NAMESPACE_BEGIN

// void SSAOPass::init()
// {
//     std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

//     attachmentsInfo[0].format = VK_FORMAT_R8_UNORM;
//     attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
//     attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//     attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//     attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

//     m_attachments.push_back(
//         Attachment(VK_FORMAT_R8_UNORM,
//                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//                    VK_IMAGE_ASPECT_COLOR_BIT,
//                    VK_IMAGE_VIEW_TYPE_2D));

//     VkAttachmentReference ref = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

//     // Subpass
//     VkSubpassDescription subpass = {};
//     subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//     subpass.colorAttachmentCount = 1;
//     subpass.pColorAttachments = &ref;

//     // Subpass dependencies for layout transitions
//     std::array<VkSubpassDependency, 2> dependencies;

//     dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//     dependencies[0].dstSubpass = 0;
//     dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//     dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
//     dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//     dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

//     dependencies[1].srcSubpass = 0;
//     dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//     dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//     dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//     dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//     dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

//     // Creation
//     VkRenderPassCreateInfo renderPassInfo{};
//     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//     renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
//     renderPassInfo.pAttachments = attachmentsInfo.data();
//     renderPassInfo.subpassCount = 1;
//     renderPassInfo.pSubpasses = &subpass;
//     renderPassInfo.dependencyCount = 2;
//     renderPassInfo.pDependencies = dependencies.data();

//     if (vkCreateRenderPass(m_context->device, &renderPassInfo, nullptr, &m_handle) != VK_SUCCESS)
//     {
//         new VKException("failed to create renderpass!");
//     }

//     m_initiatized = true;
// }

// void SSAOPass::create_descriptors()
// {

//     // Init kernel buffer

//     const size_t KERNEL_MEMBERS = 64;
//     const size_t BUFFER_SIZE = utils::pad_uniform_buffer_size(sizeof(Vec4) * KERNEL_MEMBERS, m_context->gpu);
//     m_kernelBuffer.init(m_context->memory, BUFFER_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)BUFFER_SIZE);
//     const size_t AUX_SIZE = utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_context->gpu) + utils::pad_uniform_buffer_size(sizeof(Vec2), m_context->gpu);
//     m_auxBuffer.init(m_context->memory, AUX_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)AUX_SIZE);

//     // Init and configure local descriptors
//     m_descriptorManager.init(m_context->device);
//     m_descriptorManager.create_pool(3, 1, 1, 5, 1);

//     VkDescriptorSetLayoutBinding positionTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
//     VkDescriptorSetLayoutBinding normalTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
//     VkDescriptorSetLayoutBinding noiseTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
//     VkDescriptorSetLayoutBinding kernelBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
//     VkDescriptorSetLayoutBinding auxBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
//     VkDescriptorSetLayoutBinding bindings[] = {positionTextureBinding, normalTextureBinding, noiseTextureBinding, kernelBufferBinding, auxBufferBinding};
//     m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 5);

//     m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptorSet);
// }

// void SSAOPass::create_pipelines(DescriptorManager &descriptorManager)
// {

//     // DEPTH PASS
//     ShaderPass *ssaoPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/ssao.glsl");
//     ssaoPass->settings.descriptorSetLayoutIDs =
//         {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
//          {DescriptorLayoutType::OBJECT_LAYOUT, false},
//          {DescriptorLayoutType::OBJECT_TEXTURE_LAYOUT, false}};
//     ssaoPass->settings.attributes =
//         {{VertexAttributeType::POSITION, true},
//          {VertexAttributeType::NORMAL, false},
//          {VertexAttributeType::UV, true},
//          {VertexAttributeType::TANGENT, false},
//          {VertexAttributeType::COLOR, false}};

//     ShaderPass::build_shader_stages(m_context->device, *ssaoPass);

//     ShaderPass::build(m_context->device, m_handle, m_descriptorManager, m_extent, *ssaoPass);

//     m_shaderPasses["ssao"] = ssaoPass;
// }
// void SSAOPass::init_resources()
// {

//     m_attachments[0].image.create_sampler(
//         m_context->device,
//         VK_FILTER_LINEAR,
//         VK_SAMPLER_MIPMAP_MODE_LINEAR,
//         VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
//         0.0f,
//         1.0f,
//         false,
//         1.0f,
//         VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

//     // SSAO KERNEL BUFFER -----------
//     std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
//     std::default_random_engine generator;

//     std::vector<Vec4> ssaoKernel;
//     const size_t KERNEL_MEMBERS = 64;
//     ssaoKernel.reserve(KERNEL_MEMBERS);
//     for (unsigned int i = 0; i < KERNEL_MEMBERS; ++i)
//     {
//         Vec4 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator), 0.0f);
//         sample = math::normalize(sample);
//         sample *= randomFloats(generator);
//         float scale = float(i) / 64.0f;

//         auto lerp = [](float a, float b, float f)
//         {
//             return a + f * (b - a);
//         };

//         // Center importance sampling
//         scale = lerp(0.1f, 1.0f, scale * scale);
//         sample *= scale;
//         ssaoKernel.push_back(sample);
//     }

//     const size_t BUFFER_SIZE = sizeof(Vec4) * KERNEL_MEMBERS;
//     m_kernelBuffer.upload_data(m_context->memory, ssaoKernel.data(), BUFFER_SIZE);

//     /// SSAO NOISE TEXTURE -----------
//     std::vector<Vec3> ssaoNoise;
//     for (unsigned int i = 0; i < 16; i++)
//     {
//         Vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
//         ssaoNoise.push_back(noise);
//     }
//     TextureSettings settings{};
//     settings.anisotropicFilter = false;
//     settings.useMipmaps = false;
//     m_noiseTexture = new Texture(reinterpret_cast<unsigned char *>(ssaoNoise.data()), {4, 4, 1}, settings); // Check the unsigned chars

//     Image img = m_noiseTexture->get_image();
//     m_context->upload_texture_image(img, static_cast<const void *>(ssaoNoise.data()), (VkFormat)settings.format, (VkFilter)settings.filter, (VkSamplerAddressMode)settings.adressMode, false, false);
//     m_noiseTexture->set_image(img);

//     m_descriptorManager.set_descriptor_write(m_noiseTexture->get_image().sampler, m_noiseTexture->get_image().view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 2);
//     m_descriptorManager.set_descriptor_write(&m_kernelBuffer, BUFFER_SIZE, 0, &m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3);
//     const size_t AUX_SIZE = utils::pad_uniform_buffer_size(sizeof(CameraUniforms), m_context->gpu) + utils::pad_uniform_buffer_size(sizeof(Vec2), m_context->gpu);
//     m_descriptorManager.set_descriptor_write(&m_auxBuffer, AUX_SIZE, 0, &m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4);
// }
// void SSAOPass::render(uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
// {

//       VkCommandBuffer cmd = m_context->frames[frameIndex].commandBuffer;

//     begin(cmd, presentImageIndex);

//     VkViewport viewport = init::viewport(m_extent);
//     vkCmdSetViewport(cmd, 0, 1, &viewport);
//     VkRect2D scissor{};
//     scissor.offset = {0, 0};
//     scissor.extent = m_extent;
//     vkCmdSetScissor(cmd, 0, 1, &scissor);

//     ShaderPass *shaderPass = m_shaderPasses["ssao"];

//     vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
//     vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1, &m_descriptorSet.descriptorSet, 0, VK_NULL_HANDLE);

//     Geometry *g = m_vignette->get_geometry();
//     draw(cmd, g);

//     end(cmd);
// }

// void SSAOPass::cleanup()
// {
//     RenderPass::cleanup();
//     m_descriptorManager.cleanup();
//     m_kernelBuffer.cleanup(m_context->memory);
//     m_auxBuffer.cleanup(m_context->memory);
//     m_noiseTexture->get_image().cleanup(m_context->device, m_context->memory);
// }

// void SSAOPass::set_g_buffer(Image position, Image normals)
// {

//     m_positionBuffer = position;
//     m_normalsBuffer = normals;

//     m_descriptorManager.set_descriptor_write(m_positionBuffer.sampler, m_positionBuffer.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 0);
//     m_descriptorManager.set_descriptor_write(m_normalsBuffer.sampler, m_normalsBuffer.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 1);
// }
// void SSAOPass::update_uniforms(CameraUniforms &cameraUniforms, Vec2 ssaoParams, size_t size)
// {
//     struct AuxData
//     {
//         CameraUniforms cam;
//         Vec2 ssaoParams;
//     };

//     AuxData data;
//     data.cam = cameraUniforms;
//     data.ssaoParams = ssaoParams;

//     m_auxBuffer.upload_data(m_context->memory, &data, size);
// }

// void SSAOPass::update()
// {
//     RenderPass::update();
//     m_attachments[0].image.create_sampler(
//         m_context->device,
//         VK_FILTER_LINEAR,
//         VK_SAMPLER_MIPMAP_MODE_LINEAR,
//         VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
//         0.0f,
//         1.0f,
//         false,
//         1.0f,
//         VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

//     m_descriptorManager.set_descriptor_write(m_positionBuffer.sampler, m_positionBuffer.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 0);
//     m_descriptorManager.set_descriptor_write(m_normalsBuffer.sampler, m_normalsBuffer.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 1);
// }
// VULKAN_ENGINE_NAMESPACE_END