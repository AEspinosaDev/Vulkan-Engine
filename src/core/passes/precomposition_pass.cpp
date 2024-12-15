#include <engine/core/passes/precomposition_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void PreCompositionPass::create_samples_kernel() {

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::default_random_engine              generator;

    std::vector<Vec4> AOSampleKernel;
    AOSampleKernel.resize(MAX_KERNEL_MEMBERS);

    for (unsigned int i = 0; i < m_AO.samples; ++i)
    {
        Vec4 sample(
            randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator), 0.0f);
        sample = math::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / float(m_AO.samples);

        auto lerp = [](float a, float b, float f) { return a + f * (b - a); };

        // Center importance sampling
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        AOSampleKernel[i] = sample;
    }

    m_kernelBuffer.upload_data(AOSampleKernel.data(), m_kernelBuffer.size);
    m_updateSamplesKernel = false;
}

void PreCompositionPass::setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                                           std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    // SSAO and RT SHADOWS buffer
    attachments[0] = Graphics::Attachment(RG_8U,
                                          1,
                                          LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                          COLOR_ATTACHMENT,
                                          ASPECT_COLOR,
                                          TEXTURE_2D,
                                          FILTER_LINEAR,
                                          ADDRESS_MODE_CLAMP_TO_EDGE);

    // Depdencies
    dependencies.resize(2);

    dependencies[0] = Graphics::SubPassDependency(
        STAGE_FRAGMENT_SHADER, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[0].srcAccessMask = ACCESS_SHADER_READ;
    dependencies[1] =
        Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_FRAGMENT_SHADER, ACCESS_SHADER_READ);
    dependencies[1].srcAccessMask = ACCESS_COLOR_ATTACHMENT_WRITE;
    dependencies[1].srcSubpass    = 0;
    dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
}

void PreCompositionPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {

    /////////////////////////////////////////////////
    // CREATE INTERNAL SSAO UNIFORM DATA
    /////////////////////////////////////////////////
    const size_t BUFFER_SIZE = m_device->pad_uniform_buffer_size(sizeof(Vec4) * MAX_KERNEL_MEMBERS);
    m_kernelBuffer = m_device->create_buffer_VMA(BUFFER_SIZE, BUFFER_USAGE_UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU);

    // AO KERNEL BUFFER -----------
    create_samples_kernel();

    /////////////////////////////////////////////////
    /////////////////////////////////////////////////

    m_descriptorPool = m_device->create_descriptor_pool(10, 10, 10, 10, 10);
    m_descriptors.resize(frames.size());
    // GLOBAL
    LayoutBinding camBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding sceneBufferBinding(UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_FRAGMENT, 1);
    // G- BUFFER
    LayoutBinding positionBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding normalBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    // SSAO
    LayoutBinding AOkernelBinding(UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 4);
    // RTX
    LayoutBinding noiseBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5);
    LayoutBinding accelBinding(UNIFORM_ACCELERATION_STRUCTURE, SHADER_STAGE_FRAGMENT, 6);

    m_descriptorPool.set_layout(GLOBAL_LAYOUT,
                                {camBufferBinding,
                                 sceneBufferBinding,
                                 positionBinding,
                                 normalBinding,
                                 AOkernelBinding,
                                 noiseBinding,
                                 accelBinding});

    for (size_t i = 0; i < frames.size(); i++)
    {
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);

        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(SceneUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);
        m_descriptorPool.set_descriptor_write(
            &m_kernelBuffer, BUFFER_SIZE, 0, &m_descriptors[i].globalDescritor, UNIFORM_BUFFER, 4);

        m_descriptorPool.set_descriptor_write(get_image(ResourceManager::BLUE_NOISE_TEXTURE),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              5);
    }
}
void PreCompositionPass::setup_shader_passes() {

    GraphicShaderPass* compPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, ENGINE_RESOURCES_PATH "shaders/deferred/pre_composition.glsl");
    compPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    compPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                                 {NORMAL_ATTRIBUTE, false},
                                                 {UV_ATTRIBUTE, true},
                                                 {TANGENT_ATTRIBUTE, false},
                                                 {COLOR_ATTRIBUTE, false}};

    compPass->settings.pushConstants = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(SSAOSettings))};

    compPass->build_shader_stages();
    compPass->build(m_descriptorPool);
    m_shaderPasses["pre"] = compPass;

    //   ComputeShaderPass* boxFilterPass =
    //     new ComputeShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/compute/box_filter_swap.glsl");
    // boxFilterPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};

    // boxFilterPass->build_shader_stages();
    // boxFilterPass->build(m_descriptorPool);
    // m_shaderPasses["box"] = boxFilterPass;
}

void PreCompositionPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer cmd = currentFrame.commandBuffer;

    cmd.begin_renderpass(m_renderpass, m_framebuffers[presentImageIndex]);
    cmd.set_viewport(m_renderpass.extent);

    ShaderPass* shaderPass = m_shaderPasses["pre"];

    cmd.bind_shaderpass(*shaderPass);

    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &m_AO, sizeof(SSAOSettings));
    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});

    Geometry* g = m_vignette->get_geometry();
    cmd.draw_geometry(*get_VAO(g));

    cmd.end_renderpass(m_renderpass);

    // /////////////////////////////////////////
    // /*Copy data to tmp previous frame image*/
    // /////////////////////////////////////////
    // cmd.pipeline_barrier(m_prevFrame,
    //                      LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                      LAYOUT_TRANSFER_DST_OPTIMAL,
    //                      ACCESS_SHADER_READ,
    //                      ACCESS_TRANSFER_READ,
    //                      STAGE_FRAGMENT_SHADER,
    //                      STAGE_TRANSFER);

    // cmd.blit_image(m_renderpass.attachments[0].image, m_prevFrame, FILTER_NEAREST);

    // // m_prevFrame.generate_mipmaps(cmd.handle);

    // cmd.pipeline_barrier(m_renderpass.attachments[0].image,
    //                      LAYOUT_TRANSFER_SRC_OPTIMAL,
    //                      m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                      ACCESS_TRANSFER_READ,
    //                      ACCESS_SHADER_READ,
    //                      STAGE_TRANSFER,
    //                      STAGE_FRAGMENT_SHADER);

    // cmd.pipeline_barrier(m_prevFrame,
    //                      LAYOUT_TRANSFER_DST_OPTIMAL,
    //                      LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                      ACCESS_TRANSFER_WRITE,
    //                      ACCESS_SHADER_READ,
    //                      STAGE_TRANSFER,
    //                      STAGE_FRAGMENT_SHADER);
}
void PreCompositionPass::connect_to_previous_images(std::vector<Graphics::Image> images) {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        // SET UP G-BUFFER
        m_descriptorPool.set_descriptor_write(
            &images[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 2); // POSITION
        m_descriptorPool.set_descriptor_write(
            &images[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3); // NORMALS
    }
}

void PreCompositionPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    if (m_updateSamplesKernel)
        create_samples_kernel();

    // Set STATIC top level accel. structure
    if (!get_TLAS(scene)->binded)
    {
        for (size_t i = 0; i < m_descriptors.size(); i++)
        {
            m_descriptorPool.set_descriptor_write(get_TLAS(scene), &m_descriptors[i].globalDescritor, 6);
        }
        // get_TLAS(scene)->binded = true;
    }
}

void PreCompositionPass::cleanup() {
    m_kernelBuffer.cleanup();
    GraphicPass::cleanup();
}

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END