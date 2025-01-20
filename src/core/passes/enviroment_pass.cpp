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
}
void EnviromentPass::create_framebuffer() {
    m_framebuffers[0] = m_device->create_framebuffer(m_renderpass, m_imageExtent, m_framebufferImageDepth, 0);
    m_framebuffers[1] = m_device->create_framebuffer(m_renderpass, m_irradianceResolution, m_framebufferImageDepth, 1);
}
void EnviromentPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 1);

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
    m_descriptorPool.set_descriptor_write(
        &m_framebuffers[0].attachmentImages[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_envDescriptorSet, 1);
    m_descriptorPool.set_descriptor_write(&m_captureBuffer, BUFFER_SIZE, 0, &m_envDescriptorSet, UNIFORM_BUFFER, 2);
}
void EnviromentPass::setup_shader_passes() {

    /*Conversion to Cubemap Pass*/
    // ------------------------------------
    GraphicShaderPass* converterPass =
        new GraphicShaderPass(m_device->get_handle(),
                              m_renderpass,
                              m_imageExtent,
                              ENGINE_RESOURCES_PATH "shaders/misc/panorama_converter.glsl");
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
                              ENGINE_RESOURCES_PATH "shaders/misc/irradiance_compute.glsl");
    irradiancePass->settings.descriptorSetLayoutIDs = converterPass->settings.descriptorSetLayoutIDs;
    irradiancePass->graphicSettings.attributes      = converterPass->graphicSettings.attributes;

    irradiancePass->build_shader_stages();
    irradiancePass->build(m_descriptorPool);

    m_shaderPasses["irr"] = irradiancePass;

    /*Specular Irradiance preintegration*/
    // ------------------------------------

    // TBD
}

void EnviromentPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

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
}

void EnviromentPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    if (!scene->get_skybox())
        return;
    TextureHDR* envMap = scene->get_skybox()->get_enviroment_map();
    if (envMap && envMap->loaded_on_GPU())
    {

        if (m_envDescriptorSet.bindings == 0 || envMap->is_dirty())
        {
            m_descriptorPool.set_descriptor_write(
                get_image(envMap), LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_envDescriptorSet, 0);
            envMap->set_dirty(false);
        }
    }
}

void EnviromentPass::cleanup() {
    BasePass::cleanup();
    m_captureBuffer.cleanup();
}
} // namespace Core

VULKAN_ENGINE_NAMESPACE_END