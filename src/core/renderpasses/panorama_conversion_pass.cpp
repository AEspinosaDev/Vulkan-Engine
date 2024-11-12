#include <engine/core/renderpasses/panorama_conversion_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void PanoramaConverterPass::setup_attachments() {

    m_attachments.resize(1);

    m_attachments[0] = Graphics::Attachment(static_cast<VkFormat>(m_format),
                                            VK_SAMPLE_COUNT_1_BIT,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                            AttachmentType::COLOR_ATTACHMENT,
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                            VK_IMAGE_VIEW_TYPE_CUBE,
                                            VK_FILTER_LINEAR,
                                            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    // Depdencies
    std::vector<Graphics::SubPassDependency> dependencies;
    m_dependencies.resize(1);

    m_dependencies[0] = Graphics::SubPassDependency(
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0);
}
void PanoramaConverterPass::setup_uniforms(std::vector<Graphics::Frame>& frames) {
    // Init and configure local descriptors
    m_device->create_descriptor_pool(m_descriptorPool, 1, 1, 1, 1, 1);

    LayoutBinding panoramaTextureBinding(UniformDataType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    m_descriptorPool.set_layout(DescriptorLayoutType::GLOBAL_LAYOUT, {panoramaTextureBinding});


    m_descriptorPool.allocate_descriptor_set(DescriptorLayoutType::GLOBAL_LAYOUT, &m_panoramaDescriptorSet);
}
void PanoramaConverterPass::setup_shader_passes() {

    ShaderPass* converterPass =
        new ShaderPass(m_device->get_handle(), ENGINE_RESOURCES_PATH "shaders/misc/panorama_converter.glsl");
    converterPass->settings.descriptorSetLayoutIDs = {{DescriptorLayoutType::GLOBAL_LAYOUT, true}};
    converterPass->settings.attributes             = {{VertexAttributeType::POSITION, true},
                                                      {VertexAttributeType::NORMAL, false},
                                                      {VertexAttributeType::UV, true},
                                                      {VertexAttributeType::TANGENT, false},
                                                      {VertexAttributeType::COLOR, false}};

    converterPass->build_shader_stages();
    converterPass->build(m_handle, m_descriptorPool, m_extent);

    m_shaderPasses["converter"] = converterPass;
}

void PanoramaConverterPass::render(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {

    CommandBuffer* cmd = currentFrame.commandBuffer;
    cmd->begin_renderpass(m_handle, m_framebuffers[0], m_attachments);
    cmd->set_viewport(m_extent);

    ShaderPass* shaderPass = m_shaderPasses["converter"];
    cmd->bind_shaderpass(*shaderPass);
    cmd->bind_descriptor_set(m_panoramaDescriptorSet, 0, *shaderPass);

    Geometry* g = m_vignette->get_geometry();
    cmd->draw_geometry(*get_render_data(g));
    cmd->end_renderpass();
}

void PanoramaConverterPass::update_uniforms(uint32_t frameIndex, Scene* const scene) {
    if (!scene->get_skybox())
        return;
    TextureHDR* envMap = scene->get_skybox()->get_enviroment_map();
    if (envMap && envMap->loaded_on_GPU())
    {

        if (m_panoramaDescriptorSet.bindings == 0 || envMap->is_dirty())
        {
            m_descriptorPool.set_descriptor_write(get_image(envMap)->sampler,
                                                  get_image(envMap)->view,
                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                  &m_panoramaDescriptorSet,
                                                  0);
            envMap->set_dirty(false);
        }
    }
}
void PanoramaConverterPass::connect_to_previous_images(std::vector<Image> images) {
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END