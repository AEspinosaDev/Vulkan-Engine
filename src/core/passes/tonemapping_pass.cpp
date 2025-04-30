#include <engine/core/passes/tonemapping_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Core {

void TonemappingPass::setup_input_image_mipmaps() {
    m_interAttachments[0] = m_inAttachments[0]->clone();
    m_interAttachments[0].create_view({});
}
void TonemappingPass::setup_shader_passes() {

    GraphicShaderPass* ppPass               = new GraphicShaderPass(m_device->get_handle(), m_renderpass, m_imageExtent, m_shaderPath);
    ppPass->settings.descriptorSetLayoutIDs = {{GLOBAL_LAYOUT, true}};
    ppPass->graphicSettings.attributes      = {
        {POSITION_ATTRIBUTE, true}, {NORMAL_ATTRIBUTE, false}, {UV_ATTRIBUTE, true}, {TANGENT_ATTRIBUTE, false}, {COLOR_ATTRIBUTE, false}};
    ppPass->settings.pushConstants = {Graphics::PushConstant(SHADER_STAGE_FRAGMENT, sizeof(float) * 2)};

    ppPass->build_shader_stages();
    ppPass->build(m_descriptorPool);

    m_shaderPasses["pp"] = ppPass;
}

void TonemappingPass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()

    CommandBuffer cmd = currentFrame.commandBuffer;

    // cmd.pipeline_barrier(
    //     *m_inAttachments[0], LAYOUT_SHADER_READ_ONLY_OPTIMAL, LAYOUT_TRANSFER_DST_OPTIMAL, ACCESS_SHADER_WRITE, ACCESS_TRANSFER_READ, STAGE_FRAGMENT_SHADER,
    //     STAGE_TRANSFER);
    // cmd.generate_mipmaps(*m_inAttachments[0], LAYOUT_TRANSFER_DST_OPTIMAL, LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    cmd.begin_renderpass(m_renderpass, m_framebuffers[m_isDefault ? presentImageIndex : 0]);
    cmd.set_viewport(m_imageExtent);

    ShaderPass* shaderPass = m_shaderPasses["pp"];

    cmd.bind_shaderpass(*shaderPass);
    cmd.bind_descriptor_set(m_imageDescriptorSet, 0, *shaderPass);

    struct Data {
        float exposure;
        float type;
    };
    Data data = {m_exposure, static_cast<float>(m_tonemap)};
    cmd.push_constants(*shaderPass, SHADER_STAGE_FRAGMENT, &data, sizeof(float) * 2);

    cmd.draw_geometry(*get_VAO(BasePass::vignette));

    cmd.end_renderpass(m_renderpass, m_framebuffers[m_isDefault ? presentImageIndex : 0]);
}

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END