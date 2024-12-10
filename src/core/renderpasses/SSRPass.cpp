#include <engine/core/passes/SSR_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void SSRPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    m_descriptorPool = m_device->create_descriptor_pool(
        ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS, ENGINE_MAX_OBJECTS);
    m_descriptors.resize(frames.size());

    LayoutBinding camBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding sceneBufferBinding(
        UNIFORM_DYNAMIC_BUFFER, SHADER_STAGE_VERTEX | SHADER_STAGE_GEOMETRY | SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding prevFrameBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {camBufferBinding, sceneBufferBinding, prevFrameBinding});

    // G - BUFFER SET
    LayoutBinding positionBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    LayoutBinding normalBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 1);
    LayoutBinding albedoBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 2);
    LayoutBinding materialBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 3);
    // LayoutBinding tempBinding(UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 5);
    m_descriptorPool.set_layout(1, {positionBinding, normalBinding, albedoBinding, materialBinding});

    for (size_t i = 0; i < frames.size(); i++)
    {
        // Global
        m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_descriptors[i].globalDescritor);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[0],
                                              sizeof(CameraUniforms),
                                              0,
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              0);
        m_descriptorPool.set_descriptor_write(&frames[i].uniformBuffers[0],
                                              sizeof(SceneUniforms),
                                              m_device->pad_uniform_buffer_size(sizeof(CameraUniforms)),
                                              &m_descriptors[i].globalDescritor,
                                              UNIFORM_DYNAMIC_BUFFER,
                                              1);
        // G-Buffer
        m_descriptorPool.allocate_descriptor_set(1, &m_descriptors[i].gBufferDescritor);
    }
}
void SSRPass::setup_shader_passes() {

    GraphicShaderPass* ppPass               = new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_shaderPath);
    ppPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}, {1, true}};
    ppPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                               {NORMAL_ATTRIBUTE, false},
                                               {UV_ATTRIBUTE, true},
                                               {TANGENT_ATTRIBUTE, false},
                                               {COLOR_ATTRIBUTE, false}};

    ppPass->build_shader_stages();
    ppPass->build(m_descriptorPool);

    m_shaderPasses["pp"] = ppPass;
}

void SSRPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[presentImageIndex]);
    cmd.set_viewport(m_renderpass.extent);

    ShaderPass* shaderPass = m_shaderPasses["pp"];

    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].globalDescritor, 0, *shaderPass, {0, 0});
    cmd.bind_descriptor_set(m_descriptors[currentFrame.index].gBufferDescritor, 1, *shaderPass);

    Geometry* g = m_vignette->get_geometry();
    cmd.draw_geometry(*get_VAO(g));

    // Draw gui contents
    if (m_isDefault && Frame::guiEnabled)
        cmd.draw_gui_data();

    cmd.end_renderpass();
}

void SSRPass::connect_to_previous_images(std::vector<Image> images) {
    for (size_t i = 0; i < m_descriptors.size(); i++)
    {
        // PREVIOUS FRAME
        m_descriptorPool.set_descriptor_write(
            &images[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].globalDescritor, 2);
        // SET UP G-BUFFER
        m_descriptorPool.set_descriptor_write(
            &images[1], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 0);
        m_descriptorPool.set_descriptor_write(
            &images[2], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 1);
        m_descriptorPool.set_descriptor_write(
            &images[3], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 2);
        m_descriptorPool.set_descriptor_write(
            &images[4], LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_descriptors[i].gBufferDescritor, 3);
    }
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END