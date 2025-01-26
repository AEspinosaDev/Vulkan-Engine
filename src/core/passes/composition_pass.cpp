#include <engine/core/passes/composition_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void CompositionPass::create_prev_frame_image() {

    m_prevFrame.cleanup();

    ImageConfig prevImgConfig = {};
    prevImgConfig.format      = m_colorFormat;
    prevImgConfig.usageFlags  = IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_DST | IMAGE_USAGE_TRANSFER_SRC;
    // prevImgConfig.mipLevels   = 6;
    m_prevFrame = m_device->create_image({m_imageExtent.width, m_imageExtent.height, 1}, prevImgConfig, true);
    m_prevFrame.create_view(prevImgConfig);

    SamplerConfig samplerConfig      = {};
    samplerConfig.samplerAddressMode = ADDRESS_MODE_CLAMP_TO_EDGE;
    m_prevFrame.create_sampler(samplerConfig);
}
void CompositionPass::setup_attachments(std::vector<Graphics::AttachmentInfo>&    attachments,
                                        std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(m_isDefault ? 1 : 2);

    attachments[0] = Graphics::AttachmentInfo(
        m_colorFormat,
        1,
        LAYOUT_TRANSFER_SRC_OPTIMAL,
        LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        m_isDefault ? IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_TRANSFER_SRC
                    : IMAGE_USAGE_COLOR_ATTACHMENT | IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_SRC,
        COLOR_ATTACHMENT,
        ASPECT_COLOR,
        TEXTURE_2D,
        FILTER_LINEAR,
        ADDRESS_MODE_CLAMP_TO_EDGE);

    attachments[0].isDefault = m_isDefault ? true : false;

    if (!m_isDefault) // Bright color buffer. m_colorFormat should be in floating point.
        attachments[1] = Graphics::AttachmentInfo(m_colorFormat,
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

    // Prev Frame Image
    create_prev_frame_image();
}

void CompositionPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    m_descriptorPool = m_device->create_descriptor_pool(
        ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    // GLOBAL SET
    LayoutBinding camBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding sceneBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding shadowBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding envBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding iblBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding accelBinding(UNIFORM_ACCELERATION_STRUCTURE, SHADER_STAGE_FRAGMENT, 5);
    LayoutBinding noiseBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 6);
    LayoutBinding brdfBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 7);
    LayoutBinding voxelBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 8);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT,
                                {camBufferBinding,
                                 sceneBufferBinding,
                                 shadowBinding,
                                 envBinding,
                                 iblBinding,
                                 accelBinding,
                                 noiseBinding,
                                 brdfBinding,
                                 voxelBinding});

    // G - BUFFER SET
    LayoutBinding positionBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding normalBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding albedoBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding materialBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    LayoutBinding emissionBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 4);
    LayoutBinding preCompositionBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5);
    LayoutBinding prevFrameBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 6);
    // LayoutBinding tempBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5);
    m_descriptorPool.set_layout(1,
                                {positionBinding,
                                 normalBinding,
                                 albedoBinding,
                                 materialBinding,
                                 emissionBinding,
                                 preCompositionBinding,
                                 prevFrameBinding});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.allocate_descriptor_set(1, &m_descriptors[i].gBufferDescritor);

        m_descriptorPool.update_descriptor(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.update_descriptor(&frames[i].uniformBuffers[GLOBAL_LAYOUT],
                                              sizeof(SceneUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);
        m_descriptorPool.update_descriptor(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              3);
        m_descriptorPool.update_descriptor(get_image(ResourceManager::FALLBACK_CUBEMAP),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              4);
        m_descriptorPool.update_descriptor(get_image(ResourceManager::textureResources[0]),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              6);
        m_descriptorPool.update_descriptor(get_image(ResourceManager::textureResources[1]),
                                              LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                              &m_descriptors[i].globalDescritor,
                                              7);
    }
}
void CompositionPass::setup_shader_passes() {

    GraphicShaderPass* compPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, m_imageExtent, ENGINE_RESOURCES_PATH "shaders/deferred/composition.glsl");
    compPass->settings.descriptorSetLayoutIDs  = {{GLOBAL_LAYOUT, true}, {1, true}};
    compPass->graphicSettings.attributes       = {{POSITION_ATTRIBUTE, true},
                                                  {NORMAL_ATTRIBUTE, false},
                                                  {UV_ATTRIBUTE, true},
                                                  {TANGENT_ATTRIBUTE, false},
                                                  {COLOR_ATTRIBUTE, false}};
    compPass->graphicSettings.blendAttachments = {
        Init::color_blend_attachment_state(false), Init::color_blend_attachment_state(false)};
    compPass->settings.pushConstants = {PushConstant(SHADER_STAGE_FRAGMENT, sizeof(Settings))};

    compPass->build_shader_stages();
    compPass->build(m_descriptorPool);

    m_shaderPasses["composition"] = compPass;
}

void CompositionPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;

    /*Prepare previous image for reading in case is recreated*/
    if (m_prevFrame.currentLayout == LAYOUT_UNDEFINED)
        cmd.pipeline_barrier(m_prevFrame,
                             LAYOUT_UNDEFINED,
                             LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                             ACCESS_NONE,
                             ACCESS_SHADER_READ,
                             STAGE_TOP_OF_PIPE,
                             STAGE_FRAGMENT_SHADER);

    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_imageExtent);

    ShaderPass* shaderPass = m_shaderPasses["composition"];

    cmd.bind_shaderpass(*shaderPass);

    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &m_settings, sizeof(Settings));
    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].gBufferDescritor, 1, *shaderPass);

    Geometry* g = m_vignette->get_geometry();
    cmd.draw_geometry(*get_VAO(g));

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        cmd.draw_gui_data();

    cmd.end_renderpass(m_renderpass, m_framebuffers[0]);

    /////////////////////////////////////////
    /*Copy data to tmp previous frame image*/
    /////////////////////////////////////////
    cmd.pipeline_barrier(m_prevFrame,
                         LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         LAYOUT_TRANSFER_DST_OPTIMAL,
                         ACCESS_SHADER_READ,
                         ACCESS_TRANSFER_READ,
                         STAGE_FRAGMENT_SHADER,
                         STAGE_TRANSFER);

    cmd.blit_image(m_framebuffers[0].attachmentImages[0], m_prevFrame, FILTER_NEAREST);

    // m_prevFrame.generate_mipmaps(cmd.handle);

    cmd.pipeline_barrier(m_framebuffers[0].attachmentImages[0],
                         LAYOUT_TRANSFER_SRC_OPTIMAL,
                         m_isDefault ? LAYOUT_PRESENT : LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         ACCESS_TRANSFER_READ,
                         ACCESS_SHADER_READ,
                         STAGE_TRANSFER,
                         STAGE_FRAGMENT_SHADER);

    cmd.pipeline_barrier(m_prevFrame,
                         LAYOUT_TRANSFER_DST_OPTIMAL,
                         LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         ACCESS_TRANSFER_WRITE,
                         ACCESS_SHADER_READ,
                         STAGE_TRANSFER,
                         STAGE_FRAGMENT_SHADER);
}
void CompositionPass::link_previous_images(std::vector<Graphics::Image> images) {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        // SHADOWS
        m_descriptorPool.update_descriptor(
            &images[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 2);
        // VOXELIZATION
        m_descriptorPool.update_descriptor(&images[1], LAYOUT_GENERAL, &m_descriptors[i].globalDescritor, 8);
        // SET UP G-BUFFER
        m_descriptorPool.update_descriptor(
            &images[2], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 0);
        m_descriptorPool.update_descriptor(
            &images[3], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 1);
        m_descriptorPool.update_descriptor(
            &images[4], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 2);
        m_descriptorPool.update_descriptor(
            &images[5], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 3);
        m_descriptorPool.update_descriptor(
            &images[6], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 4);
        // SSAO
        m_descriptorPool.update_descriptor(
            &images[7], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 5);
        // ENVIROMENT
        m_descriptorPool.update_descriptor(
            &images[8], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 3);
        m_descriptorPool.update_descriptor(
            &images[9], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 4);
        // PREV FRAME
        m_descriptorPool.update_descriptor(
            &m_prevFrame, LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 6);
    }
}

void CompositionPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    if (!get_TLAS(scene)->binded)
    {
        for (size_t i = 0; i < m_descriptors.size(); i++)
        {
            m_descriptorPool.update_descriptor(get_TLAS(scene), &m_descriptors[i].globalDescritor, 5);
        }
        get_TLAS(scene)->binded = true;
    }
}


void CompositionPass::update_framebuffer() {
    GraphicPass::update_framebuffer();
    create_prev_frame_image();
}

void CompositionPass::cleanup() {
    m_prevFrame.cleanup();
    GraphicPass::cleanup();
}

} // namespace Core
VULKAN_ENGINE_NAMESPACE_END