#include <engine/core/passes/enviroment_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void EnviromentPass::setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                       std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::AttachmentInfo(m_format,
                                              1,
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                              IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED,
                                              COLOR_ATTACHMENT,
                                              ASPECT_COLOR,
                                              TEXTURE_CUBE,
                                              FILTER_LINEAR,
                                              ADDRESS_MODE_CLAMP_TO_BORDER);

    // Depdencies
    dependencies.resize(1);

    dependencies[0] =
        Graphics::SubPassDependency(STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_NONE);

    m_isResizeable = false;
}
void EnviromentPass::create_framebuffer() {
    m_framebuffers[0] = m_device->create_framebuffer(m_renderpass, m_imageExtent, m_framebufferImageDepth, 0);
    m_framebuffers[1] = m_device->create_framebuffer(m_renderpass, m_irradianceResolution, m_framebufferImageDepth, 1);
}
void EnviromentPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 2);

    LayoutBinding panoramaTextureBinding(UniformDataType::UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding enviromentTextureBinding(UniformDataType::UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding auxBufferBinding(UniformDataType::UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 2);
    m_descriptorPool.set_layout(0, {panoramaTextureBinding, enviromentTextureBinding, auxBufferBinding});

    m_descriptorPool.allocate_descriptor_set(0, &m_envDescriptorSet);

    // Fill Projection Buffer
    struct CaptureData {
        Mat4 proj{math::perspective(math::radians(90.0f), 1.0f, 0.1f, 10.0f)};
        Mat4 views[CUBEMAP_FACES] = {
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(-1.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(1.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, 1.0f, 0.0f), math::vec3(0.0f, 0.0f, 1.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f), math::vec3(0.0f, 0.0f, -1.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, 0.0f, 1.0f), math::vec3(0.0f, -1.0f, 0.0f)),
            math::lookAt(math::vec3(0.0f, 0.0f, 0.0f), math::vec3(0.0f, 0.0f, -1.0f), math::vec3(0.0f, -1.0f, 0.0f))};
    };

    CaptureData capture{};

    const size_t BUFFER_SIZE = m_device->pad_uniform_buffer_size(sizeof(CaptureData));
    m_captureBuffer          = m_device->create_buffer_VMA(
        BUFFER_SIZE, BUFFER_USAGE_UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU, (uint32_t)BUFFER_SIZE);

    m_captureBuffer.upload_data(&capture, sizeof(CaptureData));

    // Set descriptors writes
    m_descriptorPool.update_descriptor(
        &m_framebuffers[0].attachmentImages[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_envDescriptorSet, 1);
    m_descriptorPool.update_descriptor(&m_captureBuffer, BUFFER_SIZE, 0, &m_envDescriptorSet, UNIFORM_BUFFER, 2);
}
void EnviromentPass::setup_shader_passes() {

    /*Conversion to Cubemap Pass*/
    // ------------------------------------
    GraphicShaderPass* converterPass =
        new GraphicShaderPass(m_device->get_handle(),
                              m_renderpass,
                              m_imageExtent,
                              ENGINE_RESOURCES_PATH "shaders/env/panorama_converter.glsl");
    converterPass->settings.descriptorSetLayoutIDs = {{0, true}};
    converterPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                                      {NORMAL_ATTRIBUTE, false},
                                                      {UV_ATTRIBUTE, true},
                                                      {TANGENT_ATTRIBUTE, false},
                                                      {COLOR_ATTRIBUTE, false}};

    converterPass->build_shader_stages();
    converterPass->build(m_descriptorPool);

    m_shaderPasses["converter"] = converterPass;

    /*Diffuse Irradiance preintegration*/
    // ------------------------------------
    GraphicShaderPass* irradiancePass =
        new GraphicShaderPass(m_device->get_handle(),
                              m_renderpass,
                              m_imageExtent,
                              ENGINE_RESOURCES_PATH "shaders/env/irradiance_compute.glsl");
    irradiancePass->settings.descriptorSetLayoutIDs = converterPass->settings.descriptorSetLayoutIDs;
    irradiancePass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                                       {NORMAL_ATTRIBUTE, false},
                                                       {UV_ATTRIBUTE, false},
                                                       {TANGENT_ATTRIBUTE, false},
                                                       {COLOR_ATTRIBUTE, false}};

    irradiancePass->build_shader_stages();
    irradiancePass->build(m_descriptorPool);

    m_shaderPasses["irr"] = irradiancePass;

    /*Specular Irradiance preintegration*/
    // ------------------------------------

    // TBD
}

void EnviromentPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    if (!scene->get_skybox())
    {
        if (m_framebuffers[1].attachmentImages[0].currentLayout == LAYOUT_UNDEFINED)
            currentFrame.commandBuffer.pipeline_barrier(
                m_framebuffers[1].attachmentImages[0], LAYOUT_UNDEFINED, LAYOUT_SHADER_READ_ONLY_OPTIMAL, ACCESS_NONE);
        return;
    }
    if (!scene->get_skybox()->update_enviroment())
        return;

    CommandBuffer cmd = currentFrame.commandBuffer;
    Geometry*     g   = m_vignette->get_geometry();

    /*Draw Cubemap*/
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);
    ShaderPass* shaderPass = m_shaderPasses["converter"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_envDescriptorSet, 0, *shaderPass);
    cmd.draw_geometry(*get_VAO(g));
    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);

    /*Draw Diffuse Irradiance*/
    cmd.begin_renderpass(m_renderpass, m_framebuffers[1]);
    cmd.set_viewport(m_irradianceResolution);
    shaderPass = m_shaderPasses["irr"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_envDescriptorSet, 0, *shaderPass);
    cmd.draw_geometry(*get_VAO(g));
    cmd.end_renderpass(m_renderpass, m_framebuffers[1]);

    scene->get_skybox()->set_update_enviroment(false);
}

void EnviromentPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    if (!scene->get_skybox())
        return;
    if (!scene->get_skybox()->update_enviroment())
        return;

    TextureHDR* envMap = scene->get_skybox()->get_enviroment_map();
    if (envMap && envMap->loaded_on_GPU())
    {
        if (envMap->is_dirty())
        {
            m_descriptorPool.update_descriptor(
                get_image(envMap), LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_envDescriptorSet, 0);
            envMap->set_dirty(false);
        }
    }
}

void EnviromentPass::update() {
    BasePass::update();

    // Update descriptor of previous framebuffer
    m_descriptorPool.update_descriptor(
        &m_framebuffers[0].attachmentImages[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_envDescriptorSet, 1);
}
void EnviromentPass::cleanup() {
    BasePass::cleanup();
    m_captureBuffer.cleanup();
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END