#include <engine/renderpasses/ssao_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

void SSAOPass::init(VkDevice &device)
{
    std::array<VkAttachmentDescription, 1> attachmentsInfo = {};

    attachmentsInfo[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachmentsInfo[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentsInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsInfo[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsInfo[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsInfo[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsInfo[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsInfo[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_attachments.push_back(
        Attachment(VK_FORMAT_R8G8B8A8_UNORM,
                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   VK_IMAGE_VIEW_TYPE_2D));

    VkAttachmentReference ref = init::attachment_reference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Creation
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsInfo.size());
    renderPassInfo.pAttachments = attachmentsInfo.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_obj) != VK_SUCCESS)
    {
        new VKException("failed to create renderpass!");
    }

    m_initiatized = true;
}

void SSAOPass::create_descriptors(VkDevice &device, VkPhysicalDevice &gpu, VmaAllocator &memory, uint32_t framesPerFlight)
{

    // Init kernel buffer
    const size_t BUFFER_SIZE = utils::pad_uniform_buffer_size(sizeof(Vec3) * KERNEL_MEMBERS, gpu);
    m_kernelBuffer.init(memory, BUFFER_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)BUFFER_SIZE);
    const size_t CAMERA_SIZE = utils::pad_uniform_buffer_size(sizeof(CameraUniforms), gpu);
    m_cameraBuffer.init(memory, CAMERA_SIZE, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)CAMERA_SIZE);

    // Init and configure local descriptors
    m_descriptorManager.init(device);
    m_descriptorManager.create_pool(10, 10, 10, 20, 10);

    VkDescriptorSetLayoutBinding positionTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    VkDescriptorSetLayoutBinding normalTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding depthTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    VkDescriptorSetLayoutBinding noiseTextureBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3);
    VkDescriptorSetLayoutBinding kernelBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4);
    VkDescriptorSetLayoutBinding cameraBufferBinding = init::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 5);
    VkDescriptorSetLayoutBinding bindings[] = {positionTextureBinding, normalTextureBinding, depthTextureBinding, noiseTextureBinding, kernelBufferBinding, cameraBufferBinding};
    m_descriptorManager.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, bindings, 6);

    m_descriptorManager.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_descriptorSet);
}

void SSAOPass::create_pipelines(VkDevice &device, DescriptorManager &descriptorManager)
{
    PipelineBuilder builder;

    // Default geometry assembly values
    builder.vertexInputInfo = init::vertex_input_state_create_info();
    builder.inputAssembly = init::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    auto bindingDescription = Vertex::getBindingDescription();
    builder.vertexInputInfo.vertexBindingDescriptionCount = 1;
    builder.vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

    // Viewport
    builder.viewport = init::viewport(m_extent);
    builder.scissor.offset = {0, 0};
    builder.scissor.extent = m_extent;

    builder.rasterizer.depthBiasEnable = VK_TRUE;
    builder.rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

    builder.depthStencil = init::depth_stencil_create_info(false, false, VK_COMPARE_OP_LESS);

    // DEPTH PASS
    ShaderPass *ssaoPass = new ShaderPass(ENGINE_RESOURCES_PATH "shaders/ssao.glsl");
    ssaoPass->settings.descriptorSetLayoutIDs =
        {{DescriptorLayoutType::GLOBAL_LAYOUT, true},
         {DescriptorLayoutType::OBJECT_LAYOUT, false},
         {DescriptorLayoutType::TEXTURE_LAYOUT, false}};
    ssaoPass->settings.attributes =
        {{VertexAttributeType::POSITION, true},
         {VertexAttributeType::NORMAL, false},
         {VertexAttributeType::UV, true},
         {VertexAttributeType::TANGENT, false},
         {VertexAttributeType::COLOR, false}};

    builder.multisampling = init::multisampling_state_create_info(VK_SAMPLE_COUNT_1_BIT);

    ShaderPass::build_shader_stages(device, *ssaoPass);

    builder.build_pipeline_layout(device, m_descriptorManager, *ssaoPass);
    builder.build_pipeline(device, m_obj, *ssaoPass);

    m_shaderPasses["ssao"] = ssaoPass;
}
void SSAOPass::init_resources(VkDevice &device,
                              VkPhysicalDevice &gpu,
                              VmaAllocator &memory,
                              VkQueue &gfxQueue,
                              utils::UploadContext &uploadContext)
{
    Geometry::upload_buffers(device, memory, gfxQueue, uploadContext, m_vignette->get_geometry());

    m_attachments[0].image.create_sampler(
        device,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        0.0f,
        1.0f,
        false,
        1.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

    // SSAO KERNEL BUFFER -----------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine generator;

    std::vector<Vec3> ssaoKernel;
    ssaoKernel.reserve(KERNEL_MEMBERS);
    for (unsigned int i = 0; i < KERNEL_MEMBERS; ++i)
    {
        Vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = math::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        auto lerp = [](float a, float b, float f)
        {
            return a + f * (b - a);
        };

        // Center importance sampling
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    const size_t BUFFER_SIZE = sizeof(Vec3) * KERNEL_MEMBERS;
    m_kernelBuffer.upload_data(memory, ssaoKernel.data(), BUFFER_SIZE);

    /// SSAO NOISE TEXTURE -----------
    std::vector<Vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        Vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    TextureSettings settings{};
    settings.anisotropicFilter = false;
    settings.useMipmaps = false;
    m_noiseTexture = new Texture(reinterpret_cast<unsigned char *>(ssaoNoise.data()), 4, 4, settings); // Check the unsigned chars
    Texture::upload_data(device, gpu, memory, gfxQueue, uploadContext, m_noiseTexture);

    m_descriptorManager.set_descriptor_write(m_positionBuffer.sampler, m_positionBuffer.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 0);
    m_descriptorManager.set_descriptor_write(m_normalsBuffer.sampler, m_normalsBuffer.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 1);
    m_descriptorManager.set_descriptor_write(m_depthBuffer.sampler, m_depthBuffer.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, &m_descriptorSet, 2);
    m_descriptorManager.set_descriptor_write(m_noiseTexture->get_image().sampler, m_noiseTexture->get_image().view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptorSet, 3);
    m_descriptorManager.set_descriptor_write(&m_kernelBuffer, BUFFER_SIZE, 0, &m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4);
    const size_t CAMERA_SIZE = utils::pad_uniform_buffer_size(sizeof(CameraUniforms), gpu);
    m_descriptorManager.set_descriptor_write(&m_cameraBuffer, CAMERA_SIZE, 0, &m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5);

}
void SSAOPass::render(Frame &frame, uint32_t frameIndex, Scene *const scene, uint32_t presentImageIndex)
{

    VkCommandBuffer cmd = frame.commandBuffer;

    begin(cmd);

    VkViewport viewport = init::viewport(m_extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    ShaderPass *shaderPass = m_shaderPasses["ssao"];

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipelineLayout, 0, 1, &m_descriptorSet.descriptorSet, 0, VK_NULL_HANDLE);
    Geometry::draw(cmd, m_vignette->get_geometry());

    end(cmd);
}

void SSAOPass::cleanup(VkDevice &device, VmaAllocator &memory)
{
    RenderPass::cleanup(device, memory);
    m_descriptorManager.cleanup();
    m_kernelBuffer.cleanup(memory);
    m_cameraBuffer.cleanup(memory);
    m_noiseTexture->get_image().cleanup(device, memory);
}

void SSAOPass::update_camera_uniforms(VmaAllocator &memory, CameraUniforms &cameraUniforms, size_t size)
{
    m_cameraBuffer.upload_data(memory, &cameraUniforms, size);
}
VULKAN_ENGINE_NAMESPACE_END