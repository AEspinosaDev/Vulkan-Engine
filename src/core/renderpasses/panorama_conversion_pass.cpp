#include <engine/core/passes/panorama_conversion_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void PanoramaConverterPass::setup_attachments(std::vector<Graphics::Attachment>&        attachments,
                                              std::vector<Graphics::SubPassDependency>& dependencies) {

    attachments.resize(1);

    attachments[0] = Graphics::Attachment(m_format,
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
void PanoramaConverterPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_descriptorPool = m_device->create_descriptor_pool(1, 1, 1, 1, 1);

    LayoutBinding panoramaTextureBinding(UniformDataType::UNIFORM_COMBINED_IMAGE_SAMPLER, SHADER_STAGE_FRAGMENT, 0);
    m_descriptorPool.set_layout(GLOBAL_LAYOUT, {panoramaTextureBinding});

    m_descriptorPool.allocate_descriptor_set(GLOBAL_LAYOUT, &m_panoramaDescriptorSet);
}
void PanoramaConverterPass::setup_shader_passes() {

    GraphicShaderPass* converterPass = new GraphicShaderPass(
        m_device->get_handle(), m_renderpass, ENGINE_RESOURCES_PATH "shaders/misc/panorama_converter.glsl");
    converterPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    converterPass->graphicSettings.attributes      = {{POSITION_ATTRIBUTE, true},
                                                      {NORMAL_ATTRIBUTE, false},
                                                      {UV_ATTRIBUTE, true},
                                                      {TANGENT_ATTRIBUTE, false},
                                                      {COLOR_ATTRIBUTE, false}};

    converterPass->build_shader_stages();
    converterPass->build(m_descriptorPool);

    m_shaderPasses["converter"] = converterPass;
}

void PanoramaConverterPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[0]);
    cmd.set_viewport(m_renderpass.extent);

    ShaderPass* shaderPass = m_shaderPasses["converter"];
    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_panoramaDescriptorSet, 0, *shaderPass);

    Geometry* g = m_vignette->get_geometry();
    cmd.draw_geometry(*get_VAO(g));
    cmd.end_renderpass(m_renderpass);
}

void PanoramaConverterPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    if (!scene->get_skybox())
        return;
    TextureHDR* envMap = scene->get_skybox()->get_enviroment_map();
    if (envMap && envMap->loaded_on_GPU())
    {

        if (m_panoramaDescriptorSet.bindings == 0 || envMap->is_dirty())
        {
            m_descriptorPool.set_descriptor_write(
                get_image(envMap), LAYOUT_SHADER_READ_ONLY_OPTIMAL, &m_panoramaDescriptorSet, 0);
            envMap->set_dirty(false);
        }
    }
}
void PanoramaConverterPass::connect_to_previous_images(std::vector<Image> images) {
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END