#include <engine/render/passes/gui_pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN
using namespace Graphics;
namespace Render {}
void Render::GUIPass::setup_out_attachments(std::vector<Graphics::AttachmentConfig>&  attachments,
                                          std::vector<Graphics::SubPassDependency>& dependencies) {
    attachments.resize(1);

    attachments[0] = Graphics::AttachmentConfig(SRGBA_8,
                                                1,
                                                LAYOUT_PRESENT,
                                                LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                IMAGE_USAGE_TRANSIENT_ATTACHMENT | IMAGE_USAGE_COLOR_ATTACHMENT,
                                                COLOR_ATTACHMENT,
                                                ASPECT_COLOR,
                                                TEXTURE_2D,
                                                FILTER_LINEAR,
                                                ADDRESS_MODE_CLAMP_TO_EDGE);

    attachments[0].loadOp        = AttachmentLoadOp::ATTACHMENT_LOAD_OP_LOAD; // Dont clear image (load from previous renderpass)
    attachments[0].initialLayout = ImageLayout::LAYOUT_PRESENT; 

    attachments[0].isDefault = true;

    m_outAttachments.resize(1); // Create dummy attachment

    dependencies.resize(1);

    dependencies[0] = Graphics::SubPassDependency(
        STAGE_COLOR_ATTACHMENT_OUTPUT, STAGE_COLOR_ATTACHMENT_OUTPUT, ACCESS_COLOR_ATTACHMENT_WRITE);
    dependencies[0].dependencyFlags = SUBPASS_DEPENDENCY_NONE;
}
void Render::GUIPass::clean_framebuffer() {
   CHECK_INITIALIZATION()
    for (Graphics::Framebuffer& fb : m_framebuffers)
        fb.cleanup();
}

void Render::GUIPass::execute(Graphics::Frame& currentFrame, Scene* const scene, uint32_t presentImageIndex) {
    PROFILING_EVENT()
    CommandBuffer cmd = currentFrame.commandBuffer;
    cmd.begin_renderpass(m_renderpass, m_framebuffers[m_isDefault ? presentImageIndex : 0]);
    
    if (Frame::guiEnabled)
        cmd.draw_gui_data();

    cmd.end_renderpass(m_renderpass, m_framebuffers[m_isDefault ? presentImageIndex : 0]);
}
// namespace Core
VULKAN_ENGINE_NAMESPACE_END